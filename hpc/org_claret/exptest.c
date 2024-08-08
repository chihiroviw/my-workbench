#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <x86intrin.h>
#include "fmath.hpp"
#define _mm256_exp_ps(x) fmath::exp_ps256(x)

#define NMAX 16384

typedef union {
  __m256 m256;
  float f[8];
} m256array;

typedef union {
  __m256i m256i;
  int i[8];
} m256iarray;

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


static void print_m256(__m256 f)
{
  m256array conv;
  int i;
  conv.m256 = f;
  for(i=0;i<8;i++) printf("%f ",conv.f[i]);
  printf("\n");
}


void exp_avx(int n, float *x, float *y)
{
  int i;
  __m256 *xa, *ya;
  xa = (__m256 *)x;
  ya = (__m256 *)y;
#pragma omp parallel for
  for(i=0;i<n/8;i++){
    ya[i] = _mm256_exp_ps(xa[i]);
  }
}


void exp_double(int n, double *x, double *y)
{
  int i;
#pragma omp parallel for
  for(i=0;i<n;i++) y[i]=exp(x[i]);
}
 

void exp_float(int n, float *x, float *y)
{
  int i;
#pragma omp parallel for
  for(i=0;i<n;i++) y[i]=expf(x[i]);
}
 

int main(int argc, char **argv)
{
  int n,nstep=1,i;
  double err,avr,ltime,stime,errone;
  double xd[NMAX],yd[NMAX],yd2[NMAX];
  float xf[NMAX]  __attribute__((aligned(32)));
  float yf[NMAX]   __attribute__((aligned(32)));

  if(argc!=3 && argc!=4){
    printf("usage : %s number_of_data calc_mode (number_of_steps)\n",argv[0]);
    printf("  calc_mode : 0 -- exp double\n");
    printf("              1 -- exp float\n");
    printf("              2 -- exp AVX (input range is from -10 to 10)\n");
    printf("              3 -- exp AVX (input range is from -300 to -100)\n");
    return 1;
  }

  // set number of particles
  sscanf(argv[1],"%d",&n);
  printf("Number of data is %d\n",n);
  if(n % 8!=0){
    fprintf(stderr,"*** error : n must be multiple of 8 **\n");
    exit(1);
  }
  if(n>NMAX){
    fprintf(stderr,"** error : too large n=%d > %d **\n",n,NMAX);
    exit(1);
  }

  // set number of steps
  if(argc>=4){
    sscanf(argv[3],"%d",&nstep);
  }
  printf("Number of steps is %d\n",nstep);

  double min,max;
  if(argv[2][0]!='3'){
    min = -10.0;
    max =  10.0;
  } else {
    min = -300.0;
    max = -100.0;
  }
  for(i=0;i<n;i++){
    xd[i]=xf[i]=min + (max-min) * drand48();
    if(i<10) printf("xd[%d]=%e\n",i,xd[i]);
  }

  // timer start
  get_cputime(&ltime,&stime);
  
  // calc with target routine
  for(i=0;i<nstep;i++){
    switch(argv[2][0]){
    case '0':
      exp_double(n,xd,yd);
      if(i==0) printf("exp double is used\n");
      break;
    case '1':
      exp_float(n,xf,yf);
      if(i==0) printf("exp float is used\n");
      break;
    case '2':
      exp_avx(n,xf,yf);
      if(i==0) printf("exp avx is used (input range is from -10 to 10)\n");
      break;
    case '3':
      exp_avx(n,xf,yf);
      if(i==0) printf("exp avx is used (input range is from -300 to -100\n");
      break;
    default:
      fprintf(stderr,"** error : cal_mode=%c is not supported **\n",argv[2][0]);
      return 1;
    }
  }
  
  // timer stop
  get_cputime(&ltime,&stime);
  printf("Time=%f seconds, %f Gexp/s\n",
	 stime,(double)n*nstep/stime/1e9);

  // check result
  if(argv[2][0]!='0'){
    for(i=0;i<n;i++) yd[i]=yf[i];
  }
  exp_double(n,xd,yd2);
  avr=0.0;
  for(i=0;i<n;i++){
    avr+=fabs(yd2[i]);
  }
  avr/=(double)n;
  for(i=0;i<3;i++){
    printf("xd=%10.3e yd[%d]=%10.3e yd2=%10.3e\n",xd[i],i,yd[i],yd2[i]);
  }
  printf("Average size of exp is %10.3e\n",avr);
  err=0.0;
  for(i=0;i<n;i++){
    errone=fabs(yd[i]-yd2[i])/avr;
    errone=fabs((yd[i]-yd2[i])/yd2[i]);
    err+=errone;
    //    if(i<16) printf("i=%d xd=%e yd=%e yd2=%e errone=%e\n",i,xd[i],yd[i],yd2[i],errone);
  }
  err/=(double)n;
  printf("Average error is %10.3e\n",err);

  return 0;
}
