#include "cuda_info.h"
#include "cuda_common.h"

// CUDA-C includes
#include <cuda.h>
#include <cuda_runtime.h>


int cudaAvaliable(void) {
  int deviceCount = 0;
  checkCudaErrors(cudaGetDeviceCount(&deviceCount));
  
  if (deviceCount == 0) {
    printf("There are no available device(s) that support CUDA\n");
  }

  return deviceCount;
}
