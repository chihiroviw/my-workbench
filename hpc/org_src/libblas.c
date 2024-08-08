#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <x86intrin.h>

void dgemm_(char *transa, char *transb, int *m, int *n_org, int *k_org,
	    double *alpha, double *A, int *ldA, double *B, int *ldB,
	    double *beta , double *C, int *ldC)
{
  int i, j, k, n = *m;
  __m256d *ai, *bj, cij;
  double t;
  if(transa[0]!='T' || transb[0]!='N' || n!=*n_org || n!=*k_org ||
     *alpha!=1.0 || *beta!=1.0 || n!=*ldA || n!=*ldB || n!=*ldC){
    fprintf(stderr,"** error : parameters are not good **\n");
    exit(1);
  }

  	// Do calculation
	#pragma omp parallel for private(j, ai, bj, cij, k)
	//#pragma omp parallel for private(j,k)
	for(i=0; i<n; i++){
		ai = (__m256d*)(A+n*i);

    		for(j=0; j<n; j++){
			bj = (__m256d*)(B+n*j);

			double *dcij = (double*)&cij;
				
			cij = _mm256_set1_pd(0.0);
      			for(k=0; k<n/4; k++){
      			//for(k=0; k<n; k++){
				//(C+n*j)[i] += (A+n*i)[k] * (B+n*j)[k];
				cij = _mm256_fmadd_pd(ai[k],bj[k],cij);
      			}
			//reduction
			(C+n*j)[i] += dcij[0]+dcij[1]+dcij[2]+dcij[3];
    		}
  	}
}
