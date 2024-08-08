#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

void dgemm_(char *transa, char *transb, int *m, int *n, int *k,
	    double *alpha, double *A, int *ldA, double *B, int *ldB,
	    double *beta , double *C, int *ldC);

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
  double **A, **B, **C, **D, *a, *b, *c, *d;
  double ltime, stime;
  int i, j, k, r, n = 100, rmax = 100;
  double alpha = 1.0, beta = 1.0, t;

  if(argc!=3){
    printf("usage : %s matrix_size number_of_repeat\n",argv[0]);
    exit(1);
  }
  sscanf(argv[1],"%d",&n);
  sscanf(argv[2],"%d",&rmax);

  // malloc
#if 1
  posix_memalign((void **)&a, 32, sizeof(double)*n*n);
  posix_memalign((void **)&b, 32, sizeof(double)*n*n);
  posix_memalign((void **)&c, 32, sizeof(double)*n*n);
  posix_memalign((void **)&d, 32, sizeof(double)*n*n);
#else
  a = (double *)malloc(sizeof(double)*n*n);
  b = (double *)malloc(sizeof(double)*n*n);
  c = (double *)malloc(sizeof(double)*n*n);
  d = (double *)malloc(sizeof(double)*n*n);
#endif
  A = (double **)malloc(sizeof(double *)*n);
  B = (double **)malloc(sizeof(double *)*n);
  C = (double **)malloc(sizeof(double *)*n);
  D = (double **)malloc(sizeof(double *)*n);
  for(i=0; i<n; i++){
    A[i] = a + n * i;
    B[i] = b + n * i;
    C[i] = c + n * i;
    D[i] = d + n * i;
  }

  // initialize matrix
  for(i=0; i<n; i++){
    for(j=0; j<n; j++){
      A[i][j] = drand48();
      B[i][j] = drand48();
      C[i][j] = 0.0;
      D[i][j] = 0.0;
    }
  }

  // timer start
  get_cputime(&ltime,&stime);

  // Do calculation
  for(r=0; r<rmax; r++){
    dgemm_("T", "N", &n, &n, &n,
	   &alpha, a,  &n, b, &n,
	   &beta , c,  &n);
  }

  // timer stop
  get_cputime(&ltime,&stime);

  // solve correct value
  for(i=0; i<n; i++){
    for(j=0; j<n; j++){
      for(k=0; k<n; k++){
	//	D[i][j] += A[i][k] * B[j][k];
	D[j][i] += A[i][k] * B[j][k];
      }
    }
  }
  for(i=0; i<n; i++){
    for(j=0; j<n; j++){
      D[i][j] *= rmax;
    }
  }

  // calc difference
  double err = 0.0;
  for(i=0; i<n; i++){
    for(j=0; j<n; j++){
      err += fabs((C[i][j]-D[i][j])/D[i][j]);
      //      printf("C[%d][%d] = %f, D=%f\n",i,j,C[i][j],D[i][j]);
    }
  }
  err /= n*n;
  
  printf("N = %d, # of repeat = %d\n",n,rmax);
  printf("Time=%f seconds, %f Gflops\n",
	 stime,2.0*n*n*n*rmax/stime/1e9);
  printf("Relative error = %e\n",err);

  // free
  free(a);
  free(b);
  free(c);
  free(d);
  free(A);
  free(B);
  free(C);
  free(D);

  return 0;
}


