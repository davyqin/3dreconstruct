#pragma once

#include <stdio.h>

typedef unsigned int uint;
typedef unsigned char uchar;

// Using shared to store computed vertices and normals during triangle generation
// improves performance
#define USE_SHARED 1

// The number of threads to use for triangle generation (limited by shared memory size)
#define NTHREADS 32

#define SKIP_EMPTY_VOXELS 0


extern "C" void initMC(int min, int max, int xyValue, int zValue, float xSpaceing, float ySpacing, float zSpacing);

extern "C" void computeIsosurface(unsigned char *volume, float x, float y, float z, 
                                  float* xResult, float* yResult, float* zResult, 
                                  uint& totalVertices);

extern "C" void cleanup();

template <class T>
void dumpBuffer(T *d_buffer, int nelements, int size_element)
{
    uint bytes = nelements * size_element;
    T *h_buffer = (T *) malloc(bytes);
    checkCudaErrors(cudaMemcpy(h_buffer, d_buffer, bytes, cudaMemcpyDeviceToHost));

    for (int i=0; i<nelements; i++)
    {
        printf("%d: %u\n", i, h_buffer[i]);
    }

    printf("\n");
    free(h_buffer);
}

template <class T>
void dumpBuffer(T *d_buffer, T *host_buffer, int nelements, int size_element)
{
  uint bytes = nelements * size_element;
  host_buffer = (T *) malloc(bytes);
  checkCudaErrors(cudaMemcpy(host_buffer, d_buffer, bytes, cudaMemcpyDeviceToHost));
}
