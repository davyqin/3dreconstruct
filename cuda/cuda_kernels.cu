#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include "cuda_kernels.h"
#include "cuda_common.h"

// Texture reference for reading image
texture<unsigned char, 2> tex;
extern __shared__ unsigned char LocalBlock[];
static cudaArray *array = NULL;

__device__ unsigned char
ComputeSobel(unsigned char ul, // upper left
             unsigned char um, // upper middle
             unsigned char ur, // upper right
             unsigned char ml, // middle left
             unsigned char mm, // middle (unused)
             unsigned char mr, // middle right
             unsigned char ll, // lower left
             unsigned char lm, // lower middle
             unsigned char lr, // lower right
             float fScale)
{
    short Horz = ur + 2*mr + lr - ul - 2*ml - ll;
    short Vert = ul + 2*um + ur - ll - 2*lm - lr;
    short Sum = (short)(fScale*(abs((int)Horz)+abs((int)Vert)));

    if (Sum < 0)
    {
        return 0;
    }
    else if (Sum > 0xff)
    {
        return 0xff;
    }

    return (unsigned char) Sum;
}

__global__ void
SobelTex(Pixel *pSobelOriginal, unsigned int Pitch, int w, int h)
{
    unsigned char *pSobel =
        (unsigned char *)(((char *) pSobelOriginal)+blockIdx.x*Pitch);

    for (int i = threadIdx.x; i < w; i += blockDim.x)
    {
        unsigned char pix00 = tex2D(tex, (float) i-1, (float) blockIdx.x-1);
        unsigned char pix01 = tex2D(tex, (float) i+0, (float) blockIdx.x-1);
        unsigned char pix02 = tex2D(tex, (float) i+1, (float) blockIdx.x-1);
        unsigned char pix10 = tex2D(tex, (float) i-1, (float) blockIdx.x+0);
        unsigned char pix11 = tex2D(tex, (float) i+0, (float) blockIdx.x+0);
        unsigned char pix12 = tex2D(tex, (float) i+1, (float) blockIdx.x+0);
        unsigned char pix20 = tex2D(tex, (float) i-1, (float) blockIdx.x+1);
        unsigned char pix21 = tex2D(tex, (float) i+0, (float) blockIdx.x+1);
        unsigned char pix22 = tex2D(tex, (float) i+1, (float) blockIdx.x+1);
        pSobel[i] = ComputeSobel(pix00, pix01, pix02,
                                 pix10, pix11, pix12,
                                 pix20, pix21, pix22, 1.0f);
    }
}

extern "C" void setupTexture(int iw, int ih, Pixel *data)
{
  cudaChannelFormatDesc desc = cudaCreateChannelDesc<unsigned char>();

  checkCudaErrors(cudaMallocArray(&array, &desc, iw, ih));
  checkCudaErrors(cudaMemcpyToArray(array, 0, 0, data, sizeof(Pixel)*iw*ih, cudaMemcpyHostToDevice));
}

// Wrapper for the __global__ call that sets up the texture and threads
extern "C" void sobelFilter(Pixel *odata, int iw, int ih)
{
  //cudaChannelFormatDesc desc = cudaCreateChannelDesc<unsigned char>();
  checkCudaErrors(cudaBindTextureToArray(tex, array));

  SobelTex<<<ih, 384>>>(odata, iw, iw, ih);

  checkCudaErrors(cudaUnbindTexture(tex));
}
