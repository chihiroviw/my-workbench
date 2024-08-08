#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MALLOC_AND_D2F(n,double_mem,float_mem) \
  (float_mem)=(float *)malloc(sizeof(float)*n);	\
  {int i;for(i=0;i<(n);i++) (float_mem)[i]=(double_mem)[i];}

void CPU_kernel(float x[], int n, int atype[], int nat,
		float pol[], float sigm[], float ipotro[],
		float pc[], float pd[], float zz[],
		int tblno, float xmax, int periodicflag,
		float force[])
{
  int i,j,k,t;
  float dn2,r,inr,inr2,inr4,inr8,d3,dr[3],fi[3];
  float pb=0.338e-19/(14.39*1.60219e-19),dphir; 

  #pragma omp parallel for private(j,k,fi,dn2,dr,r,inr,inr2,inr4,inr8,t,d3,dphir)
  for(i=0; i<n; i++){
    for(k=0; k<3; k++) fi[k] = 0.0f;

    for(j=0; j<n; j++){
      dn2 = 0.0f;

      for(k=0; k<3; k++){
	dr[k] = x[i*3+k] - x[j*3+k];
	dn2  += dr[k] * dr[k];
      }

      if(dn2 != 0.0f){
	r     = sqrtf(dn2);
	inr   = 1.0f  / r;
	inr2  = inr  * inr;
	inr4  = inr2 * inr2;
	inr8  = inr4 * inr4;
	t     = atype[i] * nat + atype[j];
	d3    = pb * pol[t] * expf( (sigm[t] - r) * ipotro[t]);
	dphir = ( d3 * ipotro[t] * inr
		  - 6.0f * pc[t] * inr8
		  - 8.0f * pd[t] * inr8 * inr2
		  + inr2 * inr * zz[t] );

	for(k=0; k<3; k++) fi[k] += dphir * dr[k];
      }

    }


    for(k=0; k<3; k++) force[i*3+k] = fi[k];
  }

}
    

void MR3calcnacl_CPU(double x[], int n, int atype[], int nat,
		     double pol[], double sigm[], double ipotro[],
		     double pc[], double pd[], double zz[],
		     int tblno, double xmax, int periodicflag,
		     double force[])
{
  int i,*f_atype;
  float *f_x,*f_pol,*f_sigm,*f_ipotro,*f_pc,*f_pd,*f_zz,*f_force,xmaxf=xmax;

  if(periodicflag!=0){
    fprintf(stderr,"** error : periodicflag = %d is not supported **\n",periodicflag);
    exit(1);
  }

  MALLOC_AND_D2F(n*3,x,f_x);
  MALLOC_AND_D2F(nat*nat,pol,f_pol);
  MALLOC_AND_D2F(nat*nat,sigm,f_sigm);
  MALLOC_AND_D2F(nat*nat,ipotro,f_ipotro);
  MALLOC_AND_D2F(nat*nat,pc,f_pc);
  MALLOC_AND_D2F(nat*nat,pd,f_pd);
  MALLOC_AND_D2F(nat*nat,zz,f_zz);
  f_force=(float *)malloc(sizeof(float)*n*3);

  CPU_kernel(f_x,n,atype,nat,f_pol,f_sigm,f_ipotro,
	     f_pc,f_pd,f_zz,tblno,xmaxf,periodicflag,f_force);
  for(i=0;i<n*3;i++) force[i]=f_force[i];

  free(f_x);
  free(f_pol);
  free(f_sigm);
  free(f_ipotro);
  free(f_pc);
  free(f_pd);
  free(f_zz);
  free(f_force);
}


