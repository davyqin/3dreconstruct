/*
 * Copyright 1993-2014 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef _MARCHING_CUBES_KERNEL_CU_
#define _MARCHING_CUBES_KERNEL_CU_

#include <stdio.h>
#include <string.h>
//#include <helper_cuda.h>    // includes for helper CUDA functions
//#include <helper_math.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>
#include <thrust/device_vector.h>
#include <thrust/scan.h>
#include <thrust/execution_policy.h>
#include <cmath>

#include <mc/common.h>

#include "cuda_common.h"
#include "helper_math.h"
#include "marching_cubes_kernel.h"

namespace {
  uint3 gridSizeShift;
  uint3 gridSize;
  uint3 gridSizeMask;

  float3 voxelSize;
  uint numVoxels = 0;
  uint maxVerts = 0;
  uint activeVoxels = 0;
  uint totalVerts = 0;

  float minIso = 0.102f;
  float maxIso = 0.902f;
  float3 startPos;

  float *d_xResult = 0;
  float *d_yResult = 0;
  float *d_zResult = 0;

  uchar *d_volume = 0;
  uint *d_voxelVerts = 0;
  uint *d_voxelVertsScan = 0;
  uint *d_voxelOccupied = 0;
  uint *d_voxelOccupiedScan = 0;
  uint *d_compVoxelArray;

  // tables
  int *d_numVertsTable = 0;
  short *d_edgeTable = 0;
  int *d_triTable = 0;


  // textures containing look-up tables
  texture<short, 1, cudaReadModeElementType> edgeTex;
  texture<int, 1, cudaReadModeElementType> triTex;
  texture<int, 1, cudaReadModeElementType> numVertsTex;

  // volume data
  texture<unsigned char, 1, cudaReadModeNormalizedFloat> volumeTex;
}

extern "C"
void allocateTextures()
{
    checkCudaErrors(cudaMalloc((void **)&d_edgeTable, 256*sizeof(short)));
    checkCudaErrors(cudaMemcpy((void *)d_edgeTable, (void *)EdgeTable, 256*sizeof(short), cudaMemcpyHostToDevice));
    cudaChannelFormatDesc channelShortDesc = cudaCreateChannelDesc(16, 0, 0, 0, cudaChannelFormatKindSigned);
    checkCudaErrors(cudaBindTexture(0, edgeTex, d_edgeTable, channelShortDesc));

    cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindSigned);
    checkCudaErrors(cudaMalloc((void **)&d_triTable, 256*16*sizeof(int)));
    checkCudaErrors(cudaMemcpy((void *)d_triTable, (void *)TriangleTable, 256*16*sizeof(int), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaBindTexture(0, triTex, d_triTable, channelDesc));

    checkCudaErrors(cudaMalloc((void **)&d_numVertsTable, 256*sizeof(int)));
    checkCudaErrors(cudaMemcpy((void *)d_numVertsTable, (void *)numVertsTable, 256*sizeof(int), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaBindTexture(0, numVertsTex, d_numVertsTable, channelDesc));
}

// sample volume data set at a point
__device__
float sampleVolume(uchar *data, uint3 p, uint3 gridSize)
{
    p.x = min(p.x, gridSize.x - 1);
    p.y = min(p.y, gridSize.y - 1);
    p.z = min(p.z, gridSize.z - 1);
    uint i = (p.z*gridSize.x*gridSize.y) + (p.y*gridSize.x) + p.x;
    //    return (float) data[i] / 255.0f;
    return tex1Dfetch(volumeTex, i);
}

// compute position in 3d grid from 1d index
// only works for power of 2 sizes
__device__
uint3 calcGridPos(uint i, uint3 gridSizeShift, uint3 gridSizeMask)
{
    uint3 gridPos;
    gridPos.x = i & gridSizeMask.x;
    gridPos.y = (i >> gridSizeShift.y) & gridSizeMask.y;
    gridPos.z = (i >> gridSizeShift.z) & gridSizeMask.z;
    return gridPos;
}

// classify voxel based on number of vertices it will generate
// one thread per voxel
__global__ void
classifyVoxel(uint *voxelVerts, uint *voxelOccupied, uchar *volume,
              uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask, uint numVoxels,
              float3 voxelSize, float minIso, float maxIso)
{
    uint blockId = __mul24(blockIdx.y, gridDim.x) + blockIdx.x;
    uint i = __mul24(blockId, blockDim.x) + threadIdx.x;

    uint3 gridPos = calcGridPos(i, gridSizeShift, gridSizeMask);
    __syncthreads();

    // read field values at neighbouring grid vertices
    float field[8];
    field[0] = sampleVolume(volume, gridPos, gridSize);
    field[1] = sampleVolume(volume, gridPos + make_uint3(1, 0, 0), gridSize);
    field[2] = sampleVolume(volume, gridPos + make_uint3(1, 1, 0), gridSize);
    field[3] = sampleVolume(volume, gridPos + make_uint3(0, 1, 0), gridSize);
    field[4] = sampleVolume(volume, gridPos + make_uint3(0, 0, 1), gridSize);
    field[5] = sampleVolume(volume, gridPos + make_uint3(1, 0, 1), gridSize);
    field[6] = sampleVolume(volume, gridPos + make_uint3(1, 1, 1), gridSize);
    field[7] = sampleVolume(volume, gridPos + make_uint3(0, 1, 1), gridSize);

    // calculate flag indicating if each vertex is inside or outside isosurface
    uint cubeindex;
    cubeindex =  uint(field[0] >= minIso && field[0] <= maxIso);
    cubeindex += uint(field[1] >= minIso && field[1] <= maxIso)*2;
    cubeindex += uint(field[2] >= minIso && field[2] <= maxIso)*4;
    cubeindex += uint(field[3] >= minIso && field[3] <= maxIso)*8;
    cubeindex += uint(field[4] >= minIso && field[4] <= maxIso)*16;
    cubeindex += uint(field[5] >= minIso && field[5] <= maxIso)*32;
    cubeindex += uint(field[6] >= minIso && field[6] <= maxIso)*64;
    cubeindex += uint(field[7] >= minIso && field[7] <= maxIso)*128;

    __syncthreads();

    // read number of vertices from texture
    uint numVerts = tex1Dfetch(numVertsTex, cubeindex);

    if (i < numVoxels)
    {
        voxelVerts[i] = numVerts;
        voxelOccupied[i] = (numVerts > 0);
    }

    __syncthreads();
}

extern "C" void
launch_classifyVoxel(dim3 grid, dim3 threads, uint *voxelVerts, uint *voxelOccupied, uchar *volume,
                     uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask, uint numVoxels,
                     float3 voxelSize)
{
    // calculate number of vertices need per voxel
    classifyVoxel<<<grid, threads>>>(voxelVerts, voxelOccupied, volume,
                                     gridSize, gridSizeShift, gridSizeMask,
                                     numVoxels, voxelSize, minIso, maxIso);
    getLastCudaError("classifyVoxel failed");
}

// compact voxel array
__global__ void
compactVoxels(uint *compactedVoxelArray, uint *voxelOccupied, uint *voxelOccupiedScan, uint numVoxels)
{
    uint blockId = __mul24(blockIdx.y, gridDim.x) + blockIdx.x;
    uint i = __mul24(blockId, blockDim.x) + threadIdx.x;

    if (voxelOccupied[i] && (i < numVoxels))
    {
        compactedVoxelArray[ voxelOccupiedScan[i] ] = i;
    }
}

extern "C" void
launch_compactVoxels(dim3 grid, dim3 threads, uint *compactedVoxelArray, uint *voxelOccupied, uint *voxelOccupiedScan, uint numVoxels)
{
    compactVoxels<<<grid, threads>>>(compactedVoxelArray, voxelOccupied,
                                     voxelOccupiedScan, numVoxels);
    getLastCudaError("compactVoxels failed");
}

// compute interpolated vertex along an edge
__device__
float3 vertexInterp(float minIso, float maxIso, float3 p0, float3 p1, float f0, float f1)
{
    float dv = f0 - f1;
    float dValMax1 = f0 - maxIso;
    float dMaxVal2 = maxIso - f1;
    float dValMin1 = f0 - minIso;
    float dMinVal2 = minIso - f1;

    if (f0 >= minIso && f0 <= maxIso) {
      if (f1 > maxIso) {
        return (p1 * dValMax1 + p0 * dMaxVal2) / dv;
      }
      else {
        return (p1 * dValMin1 + p0 * dMinVal2) / dv;
      }
    }
    else if (f0 > maxIso) {
      return (p1 * dValMax1 + p0 * dMaxVal2) / dv;
    }
    else {
      return (p1 * dValMin1 + p0 * dMinVal2) / dv;
    }
}

// version that calculates flat surface normal for each triangle
__global__ void
generateTriangles2(uint *numVertsScanned, uint *compactedVoxelArray, uchar *volume, uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask,
                   float3 voxelSize, float minIso, float maxIso, uint activeVoxels, uint maxVerts, float3 startPos,
                   float* xPos, float* yPos, float* zPos)
{
    uint blockId = __mul24(blockIdx.y, gridDim.x) + blockIdx.x;
    uint i = __mul24(blockId, blockDim.x) + threadIdx.x;

    if (i > activeVoxels - 1)
    {
        i = activeVoxels - 1;
    }

#if SKIP_EMPTY_VOXELS
    uint voxel = compactedVoxelArray[i];
#else
    uint voxel = i;
#endif

    uint3 gridPos = calcGridPos(voxel, gridSizeShift, gridSizeMask);

    float3 p;
    p.x = startPos.x + (gridPos.x * voxelSize.x);
    p.y = startPos.y + (gridPos.y * voxelSize.y);
    p.z = startPos.z + (gridPos.z * voxelSize.z);

    // calculate cell vertex positions
    float3 v[8];
    v[0] = p;
    v[1] = p + make_float3(voxelSize.x, 0, 0);
    v[2] = p + make_float3(voxelSize.x, voxelSize.y, 0);
    v[3] = p + make_float3(0, voxelSize.y, 0);
    v[4] = p + make_float3(0, 0, voxelSize.z);
    v[5] = p + make_float3(voxelSize.x, 0, voxelSize.z);
    v[6] = p + make_float3(voxelSize.x, voxelSize.y, voxelSize.z);
    v[7] = p + make_float3(0, voxelSize.y, voxelSize.z);

    float field[8];
    field[0] = sampleVolume(volume, gridPos, gridSize);
    field[1] = sampleVolume(volume, gridPos + make_uint3(1, 0, 0), gridSize);
    field[2] = sampleVolume(volume, gridPos + make_uint3(1, 1, 0), gridSize);
    field[3] = sampleVolume(volume, gridPos + make_uint3(0, 1, 0), gridSize);
    field[4] = sampleVolume(volume, gridPos + make_uint3(0, 0, 1), gridSize);
    field[5] = sampleVolume(volume, gridPos + make_uint3(1, 0, 1), gridSize);
    field[6] = sampleVolume(volume, gridPos + make_uint3(1, 1, 1), gridSize);
    field[7] = sampleVolume(volume, gridPos + make_uint3(0, 1, 1), gridSize);

    // recalculate flag
    uint cubeindex;
    cubeindex =  uint(field[0] >= minIso && field[0] <= maxIso);
    cubeindex += uint(field[1] >= minIso && field[1] <= maxIso)*2;
    cubeindex += uint(field[2] >= minIso && field[2] <= maxIso)*4;
    cubeindex += uint(field[3] >= minIso && field[3] <= maxIso)*8;
    cubeindex += uint(field[4] >= minIso && field[4] <= maxIso)*16;
    cubeindex += uint(field[5] >= minIso && field[5] <= maxIso)*32;
    cubeindex += uint(field[6] >= minIso && field[6] <= maxIso)*64;
    cubeindex += uint(field[7] >= minIso && field[7] <= maxIso)*128;

 #if USE_SHARED
    // use shared memory to avoid using local
    __shared__ float3 vertlist[12*NTHREADS];

    vertlist[threadIdx.x] = vertexInterp(minIso, maxIso, v[0], v[1], field[0], field[1]);
    vertlist[NTHREADS+threadIdx.x] = vertexInterp(minIso, maxIso, v[1], v[2], field[1], field[2]);
    vertlist[(NTHREADS*2)+threadIdx.x] = vertexInterp(minIso, maxIso, v[2], v[3], field[2], field[3]);
    vertlist[(NTHREADS*3)+threadIdx.x] = vertexInterp(minIso, maxIso, v[3], v[0], field[3], field[0]);
    vertlist[(NTHREADS*4)+threadIdx.x] = vertexInterp(minIso, maxIso, v[4], v[5], field[4], field[5]);
    vertlist[(NTHREADS*5)+threadIdx.x] = vertexInterp(minIso, maxIso, v[5], v[6], field[5], field[6]);
    vertlist[(NTHREADS*6)+threadIdx.x] = vertexInterp(minIso, maxIso, v[6], v[7], field[6], field[7]);
    vertlist[(NTHREADS*7)+threadIdx.x] = vertexInterp(minIso, maxIso, v[7], v[4], field[7], field[4]);
    vertlist[(NTHREADS*8)+threadIdx.x] = vertexInterp(minIso, maxIso, v[0], v[4], field[0], field[4]);
    vertlist[(NTHREADS*9)+threadIdx.x] = vertexInterp(minIso, maxIso, v[1], v[5], field[1], field[5]);
    vertlist[(NTHREADS*10)+threadIdx.x] = vertexInterp(minIso, maxIso, v[2], v[6], field[2], field[6]);
    vertlist[(NTHREADS*11)+threadIdx.x] = vertexInterp(minIso, maxIso, v[3], v[7], field[3], field[7]);
    __syncthreads();

#else

    // find the vertices where the surface intersects the cube
    float3 vertlist[12];

    vertlist[0] = vertexInterp(minIso, maxIso, v[0], v[1], field[0], field[1]);
    vertlist[1] = vertexInterp(minIso, maxIso, v[1], v[2], field[1], field[2]);
    vertlist[2] = vertexInterp(minIso, maxIso, v[2], v[3], field[2], field[3]);
    vertlist[3] = vertexInterp(minIso, maxIso, v[3], v[0], field[3], field[0]);

    vertlist[4] = vertexInterp(minIso, maxIso, v[4], v[5], field[4], field[5]);
    vertlist[5] = vertexInterp(minIso, maxIso, v[5], v[6], field[5], field[6]);
    vertlist[6] = vertexInterp(minIso, maxIso, v[6], v[7], field[6], field[7]);
    vertlist[7] = vertexInterp(minIso, maxIso, v[7], v[4], field[7], field[4]);

    vertlist[8] = vertexInterp(minIso, maxIso, v[0], v[4], field[0], field[4]);
    vertlist[9] = vertexInterp(minIso, maxIso, v[1], v[5], field[1], field[5]);
    vertlist[10] = vertexInterp(minIso, maxIso, v[2], v[6], field[2], field[6]);
    vertlist[11] = vertexInterp(minIso, maxIso, v[3], v[7], field[3], field[7]);
#endif

    // output triangle vertices
    uint numVerts = tex1Dfetch(numVertsTex, cubeindex);

    for (int i = 0; i < numVerts; i+=3)
    {
        uint index = numVertsScanned[voxel] + i;

        float3 v[3];
        int edge;
        edge = tex1Dfetch(triTex, (cubeindex*16) + i);
#if USE_SHARED
        v[0] = vertlist[(edge*NTHREADS)+threadIdx.x];
#else
        v[0] = vertlist[edge];
#endif

        edge = tex1Dfetch(triTex, (cubeindex*16) + i + 1);
#if USE_SHARED
        v[1] = vertlist[(edge*NTHREADS)+threadIdx.x];
#else
        v[1] = vertlist[edge];
#endif


        edge = tex1Dfetch(triTex, (cubeindex*16) + i + 2);
#if USE_SHARED
        v[2] = vertlist[(edge*NTHREADS)+threadIdx.x];
#else
        v[2] = vertlist[edge];
#endif

        if (index < (maxVerts - 3))
        {
          xPos[index] = v[0].x;
          yPos[index] = v[0].y;
          zPos[index] = v[0].z;

          xPos[index+1] = v[1].x;
          yPos[index+1] = v[1].y;
          zPos[index+1] = v[1].z;

          xPos[index+2] = v[2].x;
          yPos[index+2] = v[2].y;
          zPos[index+2] = v[2].z;
        }
    }
}

extern "C" void
launch_generateTriangles2(dim3 grid, dim3 threads, uint *numVertsScanned, uint *compactedVoxelArray, uchar *volume,
                          uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask,
                          float3 voxelSize, uint activeVoxels, uint maxVerts)
{
    generateTriangles2<<<grid, NTHREADS>>>(numVertsScanned, compactedVoxelArray, volume,
                                           gridSize, gridSizeShift, gridSizeMask,
                                           voxelSize, minIso, maxIso, activeVoxels,
                                           maxVerts, startPos, d_xResult, d_yResult, d_zResult);
    getLastCudaError("generateTriangles2 failed");
}

extern "C" void ThrustScanWrapper(unsigned int *output, unsigned int *input, unsigned int numElements)
{
  thrust::exclusive_scan(thrust::device_ptr<unsigned int>(input),
                         thrust::device_ptr<unsigned int>(input + numElements),
                         thrust::device_ptr<unsigned int>(output));
}

extern "C" void initMC(int min, int max, int xyValue, int zValue, float xSpaceing, float ySpacing, float zSpacing)
{
  allocateTextures();

  minIso = (float)(min)/255.0f;
  maxIso = (float)(max)/255.0f;
  gridSize = make_uint3(xyValue, xyValue, zValue);
  gridSizeMask = make_uint3(gridSize.x-1, gridSize.y-1, gridSize.z-1);
  const int logBase2 = log10(xyValue) / log10(2);
  gridSizeShift = make_uint3(0, logBase2, logBase2*2);

  numVoxels = gridSize.x*gridSize.y*(gridSize.z - 1);
  voxelSize = make_float3(xSpaceing, ySpacing, zSpacing);
  maxVerts = gridSize.x * gridSize.y * 50;

  int size = gridSize.x*gridSize.y*gridSize.z*sizeof(uchar);
  checkCudaErrors(cudaMalloc((void **) &d_volume, size));

  // allocate device memory
  unsigned int memSize = sizeof(uint) * numVoxels;
  checkCudaErrors(cudaMalloc((void **) &d_voxelVerts,            memSize));
  checkCudaErrors(cudaMalloc((void **) &d_voxelVertsScan,        memSize));
  checkCudaErrors(cudaMalloc((void **) &d_voxelOccupied,         memSize));
  checkCudaErrors(cudaMalloc((void **) &d_voxelOccupiedScan,     memSize));
  checkCudaErrors(cudaMalloc((void **) &d_compVoxelArray,        memSize));
  checkCudaErrors(cudaMalloc((void **) &(d_xResult),             maxVerts*sizeof(float)));
  checkCudaErrors(cudaMalloc((void **) &(d_yResult),             maxVerts*sizeof(float)));
  checkCudaErrors(cudaMalloc((void **) &(d_zResult),             maxVerts*sizeof(float)));
}

extern "C"
void bindVolumeTexture(unsigned char *volume)
{
  int size = gridSize.x*gridSize.y*gridSize.z*sizeof(uchar);
  checkCudaErrors(cudaMemcpy(d_volume, volume, size, cudaMemcpyHostToDevice));
  
  // bind to linear texture
  checkCudaErrors(cudaBindTexture(0, volumeTex, d_volume, cudaCreateChannelDesc(8, 0, 0, 0, cudaChannelFormatKindUnsigned)));
}

extern "C" void computeIsosurface(unsigned char *volume, float x, float y, float z, float* xResult, float* yResult, float* zResult, uint& totalVertices)
{
  bindVolumeTexture(volume);

  startPos.x = x;
  startPos.y = y;
  startPos.z = z;

  int threads = 128;
  dim3 grid(numVoxels / threads, 1, 1);

 // get around maximum grid size of 65535 in each dimension
  if (grid.x > 65535)
  {
    grid.y = grid.x / 32768;
    grid.x = 32768;
  }

  // calculate number of vertices need per voxel
  launch_classifyVoxel(grid, threads,
                       d_voxelVerts, d_voxelOccupied, d_volume,
                       gridSize, gridSizeShift, gridSizeMask,
                       numVoxels, voxelSize);

#if SKIP_EMPTY_VOXELS
#if _DEBUG
    {
    uint* voxelOccupied = new uint[numVoxels];
    checkCudaErrors(cudaMemcpy(voxelOccupied, d_voxelOccupied, numVoxels * sizeof(uint), cudaMemcpyDeviceToHost));
  
    uint* voxelOccupiedScan = new uint[numVoxels+1];
    voxelOccupiedScan[0] = 0;
    for (int i = 1; i <= numVoxels; ++i) {
      voxelOccupiedScan[i] = voxelOccupiedScan[i - 1] + voxelOccupied[i -1];
    }

    activeVoxels = voxelOccupiedScan[numVoxels];
    checkCudaErrors(cudaMemcpy(d_voxelOccupiedScan, voxelOccupiedScan, numVoxels * sizeof(uint), cudaMemcpyHostToDevice));

    delete [] voxelOccupied;
    delete [] voxelOccupiedScan;
  }
#else

    // scan voxel occupied array
    ThrustScanWrapper(d_voxelOccupiedScan, d_voxelOccupied, numVoxels);

    // read back values to calculate total number of non-empty voxels
    // since we are using an exclusive scan, the total is the last value of
    // the scan result plus the last value in the input array
    {
        uint lastElement, lastScanElement;
        checkCudaErrors(cudaMemcpy((void *) &lastElement,
                                   (void *)(d_voxelOccupied + numVoxels-1),
                                   sizeof(uint), cudaMemcpyDeviceToHost));
        checkCudaErrors(cudaMemcpy((void *) &lastScanElement,
                                   (void *)(d_voxelOccupiedScan + numVoxels-1),
                                   sizeof(uint), cudaMemcpyDeviceToHost));
        activeVoxels = lastElement + lastScanElement;
    }
#endif
    if (activeVoxels==0)
    {
        // return if there are no full voxels
        totalVerts = 0;
        return;
    }

    // compact voxel index array
    launch_compactVoxels(grid, threads, d_compVoxelArray, d_voxelOccupied, d_voxelOccupiedScan, numVoxels);
    getLastCudaError("compactVoxels failed");

#endif // SKIP_EMPTY_VOXELS

#if _DEBUG
  
  uint* voxelVerts = new uint[numVoxels];
  checkCudaErrors(cudaMemcpy(voxelVerts, d_voxelVerts, numVoxels * sizeof(uint), cudaMemcpyDeviceToHost));
  
  uint* voxelVertsScan = new uint[numVoxels+1];
  voxelVertsScan[0] = 0;
  for (int i = 1; i <= numVoxels; ++i) {
    voxelVertsScan[i] = voxelVertsScan[i - 1] + voxelVerts[i -1];
  }

  totalVerts = voxelVertsScan[numVoxels];
  totalVertices = totalVerts;
  checkCudaErrors(cudaMemcpy(d_voxelVertsScan, voxelVertsScan, numVoxels * sizeof(uint), cudaMemcpyHostToDevice));

  delete [] voxelVerts;
  delete [] voxelVertsScan;
  
#else
   // scan voxel vertex count array
  ThrustScanWrapper(d_voxelVertsScan, d_voxelVerts, numVoxels);

  // readback total number of vertices
  {
      uint lastElement, lastScanElement;
      checkCudaErrors(cudaMemcpy((void *) &lastElement,
                                  (void *)(d_voxelVerts + numVoxels-1),
                                  sizeof(uint), cudaMemcpyDeviceToHost));
      checkCudaErrors(cudaMemcpy((void *) &lastScanElement,
                                  (void *)(d_voxelVertsScan + numVoxels-1),
                                  sizeof(uint), cudaMemcpyDeviceToHost));
      totalVerts = lastElement + lastScanElement;
      totalVertices = totalVerts;
  }
#endif

  dim3 grid2((int) ceil(numVoxels / (float) NTHREADS), 1, 1);

  while (grid2.x > 65535)
  {
    grid2.x/=2;
    grid2.y*=2;
  }

  launch_generateTriangles2(grid2, NTHREADS, d_voxelVertsScan, d_compVoxelArray, d_volume,
                            gridSize, gridSizeShift, gridSizeMask,
                            voxelSize, activeVoxels, maxVerts);

  checkCudaErrors(cudaMemcpy(xResult, d_xResult, maxVerts * sizeof(float), cudaMemcpyDeviceToHost));
  checkCudaErrors(cudaMemcpy(yResult, d_yResult, maxVerts * sizeof(float), cudaMemcpyDeviceToHost));
  checkCudaErrors(cudaMemcpy(zResult, d_zResult, maxVerts * sizeof(float), cudaMemcpyDeviceToHost));
}

extern "C" void cleanup()
{
  checkCudaErrors(cudaFree(d_edgeTable));
  checkCudaErrors(cudaFree(d_triTable));
  checkCudaErrors(cudaFree(d_numVertsTable));

  checkCudaErrors(cudaFree(d_voxelVerts));
  checkCudaErrors(cudaFree(d_voxelVertsScan));
  checkCudaErrors(cudaFree(d_voxelOccupied));
  checkCudaErrors(cudaFree(d_voxelOccupiedScan));
  checkCudaErrors(cudaFree(d_compVoxelArray));
  checkCudaErrors(cudaFree(d_xResult));
  checkCudaErrors(cudaFree(d_yResult));
  checkCudaErrors(cudaFree(d_zResult));

  if (d_volume)
  {
    checkCudaErrors(cudaFree(d_volume));
  }
}

#endif
