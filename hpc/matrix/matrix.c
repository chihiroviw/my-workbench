#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

static void get_cputime(double *laptime, double *sprittime)
{
  struct timeval tv;
  struct timezone tz;
  double sec,microsec;

  gettimeofday(&tv, &tz);
  sec=tv.tv_sec;
  microsec=tv.tv_usec;

  *sprittime = sec + microsec * 1e-6 - *laptime;
  *laptime = sec + microsec * 1e-6;
}


int main(int argc, char **argv)
{
  double **A, **B, **C, *a, *b, *c;
  double ltime, stime;
  int i, j, k, r, n = 100, rmax = 100;

  // malloc
  a = (double *)malloc(sizeof(double)*n*n);
  b = (double *)malloc(sizeof(double)*n*n);
  c = (double *)malloc(sizeof(double)*n*n);
  A = (double **)malloc(sizeof(double *)*n);
  B = (double **)malloc(sizeof(double *)*n);
  C = (double **)malloc(sizeof(double *)*n);
  for(i=0; i<n; i++){
    A[i] = a + n * i;
    B[i] = b + n * i;
    C[i] = c + n * i;
  }

  // initialize matrix
  for(i=0; i<n; i++){
    for(j=0; j<n; j++){
      A[i][j] = drand48();
      B[i][j] = drand48();
      C[i][j] = 0.0;
    }
  }

  // timer start
  get_cputime(&ltime,&stime);

  // Do calculation
  for(r=0; r<rmax; r++){
	for(i=0; i<n; i++){
		for(j=0; j<n; j++){
			for(k=0; k<n; k++){
				C[i][j] += A[i][k] + B[j][k];
			}
		}
	}
  } 
  // timer stop
  get_cputime(&ltime,&stime);
  printf("N = %d, # of repeat = %d\n",n,rmax);
  printf("Time=%f seconds, %f Gflops\n",
	 stime,2.0*n*n*n*rmax/stime/1e9);

  // free
  free(a);
  free(b);
  free(c);
  free(A);
  free(B);
  free(C);

  return 0;
}


