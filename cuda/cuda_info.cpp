#include "cuda_info.h"
#include "cuda_common.h"

// CUDA-C includes
#include <cuda.h>
#include <cuda_runtime.h>


int cudaAvaliable(void) {
  int deviceCount = 0;
  cudaError_t error_id = cudaGetDeviceCount(&deviceCount);
  
  if (error_id != cudaSuccess || deviceCount == 0) {
    printf("There are no available device(s) that support CUDA\n");
  }

  return deviceCount;
}
