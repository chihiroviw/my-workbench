#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define EPS2    (0.03f*0.03f)
#define NTHRE   (64)

__global__
void float_force_optimized_sub(int n,                // input  : number of particles
			       float posf[][4],      // input  : position and mass of particles
			       float forcef[][4])    // output : force and potential of particles
{
  int tid = threadIdx.x;
  int i = blockIdx.x * NTHRE + tid;
  int j,k;
  float dr[3],r_1,dtmp,r2,fi[4],sqrtfeps2=1.0f/sqrtf(EPS2);
  if(i<n){
	for(k=0;k<4;k++) fi[k]=0.0f;
	for(j=0;j<n;j++){
		r2=EPS2;

		for(k=0;k<3;k++){
			dr[k]=posf[i][k]-posf[j][k];
			r2+=dr[k]*dr[k];
		}

		r_1=1.0f/sqrtf(r2);
	        dtmp=posf[j][3]*r_1;
	        fi[3]+=dtmp;
	        dtmp*=r_1*r_1;

	        for(k=0;k<3;k++) fi[k]-=dtmp*dr[k];
	}

	fi[3]-=posf[i][3]*sqrtfeps2;

	for(k=0;k<4;k++) forcef[i][k]=fi[k]*posf[i][3];
  }
}

/*
__global__
void float_force_optimized_sub(int n,                // input  : number of particles
			       float posf[][4],      // input  : position and mass of particles
			       float forcef[][4])    // output : force and potential of particles
{
  int tid = threadIdx.x;
  int i = blockIdx.x * NTHRE + tid;
  int j,k;
  float dr[3],r_1,dtmp,r2,fi[4],sqrtfeps2=1.0f/sqrtf(EPS2);

  if(i<n){
    for(k=0;k<4;k++) fi[k]=0.0f;
    for(j=0;j<n;j++){
      r2=EPS2;
      for(k=0;k<3;k++){
	dr[k]=posf[i][k]-posf[j][k];
	r2+=dr[k]*dr[k];
      }
      r_1=1.0f/sqrtf(r2);
      dtmp=posf[j][3]*r_1;
      fi[3]+=dtmp;
      dtmp*=r_1*r_1;
      for(k=0;k<3;k++) fi[k]-=dtmp*dr[k];
    }
    fi[3]-=posf[i][3]*sqrtfeps2;
    for(k=0;k<4;k++) forcef[i][k]=fi[k]*posf[i][3];
  }
}
*/


extern "C"
void float_force_optimized(int n,                // input  : number of particles
			   double pos[][4],      // input  : position and mass of particles
			   double force[][4])    // output : force and potential of particles
{
  int i,j,k;
  float (*posf)[4],(*forcef)[4];
  float (*d_posf)[4], (*d_forcef)[4];

  // Allocate and copy positions and masses
  posf=(float (*)[4])malloc(sizeof(float)*n*4);
  forcef=(float (*)[4])malloc(sizeof(float)*n*4);
  cudaMalloc((void **)&d_posf,sizeof(float)*n*4);
  cudaMalloc((void **)&d_forcef,sizeof(float)*n*4);
  for(j=0;j<n;j++) for(k=0;k<4;k++) posf[j][k]=(float)pos[j][k];
  cudaMemcpy(d_posf,posf,sizeof(float)*n*4,cudaMemcpyHostToDevice);

  // Calculate force and potential with GPU
  dim3 threads(NTHRE);
  dim3 grid((n+NTHRE-1)/NTHRE);
  float_force_optimized_sub<<<grid, threads>>>(n,d_posf,d_forcef);

  // Copy back to double array
  cudaMemcpy(forcef,d_forcef,sizeof(float)*n*4,cudaMemcpyDeviceToHost);
  for(i=0;i<n;i++) for(k=0;k<4;k++) force[i][k]=(double)forcef[i][k];

  free(posf);free(forcef);
  cudaFree(d_posf);cudaFree(d_forcef);
}

