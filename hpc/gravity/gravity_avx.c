#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <x86intrin.h>

#define EPS2    (0.03f*0.03f)

static void float_force_optimized_sub(int n,                // input  : number of particles
				      float posf[][4],      // input  : position and mass of particles
				      float forcef[][4])    // output : force and potential of particles
{
  int i,ii,j,k;
  __m256 dr[3],r_1,dtmp,r2,fi[4],posi[4],posj[4];
  __m256 half = _mm256_set1_ps(0.5f);
  __m256 one  = _mm256_set1_ps(1.0f);
  __m256 msqrtfeps2 = _mm256_set1_ps(-1.0f/sqrtf(EPS2));

  if(n % 8!=0){
    fprintf(stderr,"** error : n must be multiple of 8 **\n");
    exit(1);
  }

#pragma omp parallel for private(k,j,r2,dtmp,r_1,dr,fi,ii,posi,posj)
  for(i=0;i<n/8;i++){
    for(k=0;k<4;k++){
      fi[k]=_mm256_set1_ps(0.0f);
      posi[k]=_mm256_setr_ps(posf[i*8][k],posf[i*8+1][k],posf[i*8+2][k],posf[i*8+3][k],
			     posf[i*8+4][k],posf[i*8+5][k],posf[i*8+6][k],posf[i*8+7][k]);
    }
    for(j=0;j<n;j++){
      for(k=0;k<4;k++) posj[k]=_mm256_set1_ps(posf[j][k]);
      r2=_mm256_set1_ps(EPS2);
      for(k=0;k<3;k++){
	dr[k]=_mm256_sub_ps(posi[k],posj[k]);
	r2 = _mm256_fmadd_ps(dr[k],dr[k],r2);
      }

      r_1 = _mm256_div_ps(one,_mm256_sqrt_ps(r2));
      /*
      r_1=_mm256_rsqrt_ps(r2);
#if 1 // Newton method
      r2 = _mm256_mul_ps(r_1,r2);
      r2 = _mm256_fnmadd_ps(r_1,r2,one);
      r2 = _mm256_mul_ps(r2,r_1);
      r_1 = _mm256_fmadd_ps(r2,half,r_1);
#endif // end of Newton method
      */

      dtmp=_mm256_mul_ps(posj[3],r_1);
      fi[3]=_mm256_add_ps(fi[3],dtmp);
      dtmp=_mm256_mul_ps(dtmp,_mm256_mul_ps(r_1,r_1));
      for(k=0;k<3;k++) fi[k]=_mm256_fnmadd_ps(dtmp,dr[k],fi[k]);
    }
    fi[3]=_mm256_fmadd_ps(posi[3],msqrtfeps2,fi[3]);
    for(k=0;k<4;k++) fi[k]=_mm256_mul_ps(fi[k],posi[3]);
    for(ii=0;ii<8;ii++){
      for(k=0;k<4;k++) forcef[i*8+ii][k]=((float *)(fi+k))[ii];
    }
  }
}


void float_force_optimized(int n,                // input  : number of particles
			   double pos[][4],      // input  : position and mass of particles
			   double force[][4])    // output : force and potential of particles
{
  int i,j,k;
  float (*posf)[4],(*forcef)[4];

  // Allocate and copy positions and masses
  posix_memalign((void **)&posf,32,sizeof(float)*n*4);
  posix_memalign((void **)&forcef,32,sizeof(float)*n*4);
  for(j=0;j<n;j++) for(k=0;k<4;k++) posf[j][k]=(float)pos[j][k];

  // Calculate force and potential with float
  float_force_optimized_sub(n,posf,forcef);

  // Copy back to double array
  for(i=0;i<n;i++) for(k=0;k<4;k++) force[i][k]=(double)forcef[i][k];

  free(posf);free(forcef);
}

