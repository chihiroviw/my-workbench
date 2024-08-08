#include <stdio.h>
#include <stdlib.h>

#define SIZE    (3072)
#define NTHRE   (64)

float in[SIZE];
float out[SIZE];

__global__ void abs_kernel(int n, float *in, float *out)
{
  /* Calculate absolute values */
  int i=blockIdx.x * blockDim.x + threadIdx.x, loop;

  for(loop=0;loop<1000000;loop++){
    if (in[i] > 0) {
      out[i] = in[i];
    } else {
      out[i] = in[i] * -1;
    }
  }
}

int main(int argc, char **argv)
{
  int i;
  float *d_in, *d_out;

  for(i=0;i<SIZE;i+=2){
    in[i]=i+1;
    in[i+1]=-(i+2);
  }

  /* allocate and copy GPU memory */
  cudaMalloc((void **)&d_in, sizeof(float)*SIZE);
  cudaMalloc((void **)&d_out,sizeof(float)*SIZE);
  cudaMemcpy(d_in,in,sizeof(float)*SIZE,cudaMemcpyHostToDevice);

  /* call GPU kernel */
  dim3 threads(NTHRE);
  dim3 grid((SIZE + NTHRE - 1)/NTHRE);
  abs_kernel<<<grid, threads>>>(SIZE, d_in, d_out);

  /* copy result to CPU memory */
  cudaMemcpy(out,d_out,sizeof(float)*SIZE,cudaMemcpyDeviceToHost);

  for (i = SIZE-3; i < SIZE; i++) {
    printf("in [%02d]=%0.0f\n", i, in[i]);
    printf("out[%02d]=%0.0f\n", i, out[i]);
  }
  cudaFree(d_in); cudaFree(d_out);  
  return 0;
}
