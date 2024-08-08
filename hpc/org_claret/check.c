/*
  for ((n=2048;n<=4096;n+=16));do ./a.out $n 2 10|egrep -e Gflops -e "Number of particle" ;done | tee log2.txt 
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

void MR3calcnacl_CPU(double x[], int n, int atype[], int nat,
		     double pol[], double sigm[], double ipotro[],
		     double pc[], double pd[], double zz[],
		     int tblno, double xmax, int periodicflag,
		     double force[]);
void MR3calcnacl_AVX(double x[], int n, int atype[], int nat,
		     double pol[], double sigm[], double ipotro[],
		     double pc[], double pd[], double zz[],
		     int tblno, double xmax, int periodicflag,
		     double force[]);
void MR3calcnacl_GPU(double x[], int n, int atype[], int nat,
		     double pol[], double sigm[], double ipotro[],
		     double pc[], double pd[], double zz[],
		     int tblno, double xmax, int periodicflag,
		     double force[]);


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


void MR3calcnacl(double x[], int n, int atype[], int nat,
		 double pol[], double sigm[], double ipotro[],
		 double pc[], double pd[], double zz[],
		 int tblno, double xmax, int periodicflag,
		 double force[])
{
  int i,j,k,t;
  double xmax1,dn2,r,inr,inr2,inr4,inr8,d3,dr[3],fi[3];
  double pb=0.338e-19/(14.39*1.60219e-19),dphir; 
  if((periodicflag & 1)==0) xmax *= 2;
  xmax1 = 1.0 / xmax;

  for(i=0; i<n; i++){
    for(k=0; k<3; k++) fi[k] = 0.0;
    for(j=0; j<n; j++){
      dn2 = 0.0;
      for(k=0; k<3; k++){
	dr[k] =  x[i*3+k] - x[j*3+k];
	dr[k] -= rint(dr[k] * xmax1) * xmax;
	dn2   += dr[k] * dr[k];
      }
      if(dn2 != 0.0){
	r     = sqrt(dn2);
	inr   = 1.0  / r;
	inr2  = inr  * inr;
	inr4  = inr2 * inr2;
	inr8  = inr4 * inr4;
	t     = atype[i] * nat + atype[j];
	d3    = pb * pol[t] * exp( (sigm[t] - r) * ipotro[t]);
	dphir = ( d3 * ipotro[t] * inr
		  - 6.0 * pc[t] * inr8
		  - 8.0 * pd[t] * inr8 * inr2
		  + inr2 * inr * zz[t] );
	for(k=0; k<3; k++) fi[k] += dphir * dr[k];
      }
    }
    for(k=0; k<3; k++) force[i*3+k] = fi[k];
  }
}
    


int main(int argc, char **argv)
{
  int i,j,n,nstep=1,nat=2,pflag=0;
  double *x,*a1,*a2;
  double *pol,*sigm,*ipotro,*pc,*pd,*zz;
  int *atype;
  double xmax=100.0;
  double ltime,stime;
  double avr,aone,err,eone;

  if(argc!=3 && argc!=4){
    printf("usage : %s number_of_particles calc_mode (number_of_steps)\n",argv[0]);
    printf("  calc_mode : 0 -- Original routine is used (double)\n");
    printf("              1 -- CPU routine is used (float)\n");
    printf("              2 -- AVX routine is used\n");
    printf("              3 -- GPU routine is used\n");
    return 1;
  }

  // set number of particles
  sscanf(argv[1],"%d",&n);
  printf("Number of particle is %d\n",n);

  // set number of steps
  if(argc==4){
    sscanf(argv[3],"%d",&nstep);
  }
  printf("Number of steps is %d\n",nstep);

  // allocate variables
  if((x=(double *)malloc(sizeof(double)*n*3))==NULL){
    fprintf(stderr,"** error : can't malloc x **\n");
    return 1;
  }
  if((a1=(double *)malloc(sizeof(double)*n*3))==NULL){
    fprintf(stderr,"** error : can't malloc a1 **\n");
    return 1;
  }
  if((a2=(double *)malloc(sizeof(double)*n*3))==NULL){
    fprintf(stderr,"** error : can't malloc a2 **\n");
    return 1;
  }
  if((atype=(int *)malloc(sizeof(int)*n))==NULL){
    fprintf(stderr,"** error : can't malloc atype **\n");
    return 1;
  }
  if((pol=(double *)malloc(sizeof(double)*nat*nat))==NULL){
    fprintf(stderr,"** error : can't malloc pol **\n");
  }
  if((sigm=(double *)malloc(sizeof(double)*nat*nat))==NULL){
    fprintf(stderr,"** error : can't malloc sigm **\n");
  }
  if((ipotro=(double *)malloc(sizeof(double)*nat*nat))==NULL){
    fprintf(stderr,"** error : can't malloc ipotro **\n");
  }
  if((pc=(double *)malloc(sizeof(double)*nat*nat))==NULL){
    fprintf(stderr,"** error : can't malloc pc **\n");
  }
  if((pd=(double *)malloc(sizeof(double)*nat*nat))==NULL){
    fprintf(stderr,"** error : can't malloc pd **\n");
  }
  if((zz=(double *)malloc(sizeof(double)*nat*nat))==NULL){
    fprintf(stderr,"** error : can't malloc zz **\n");
  }

  // set positions and types
  for(i=0;i<n;i++){
    for(j=0;j<3;j++){
      x[i*3+j]=drand48()*xmax;
    }
    atype[i]=drand48()*nat;
  }

  // set parameters between atoms
  for(i=0;i<nat;i++){
    for(j=0;j<nat;j++){
      pol[i*nat+j]=1.0+drand48();
      sigm[i*nat+j]=2.0+drand48();
      ipotro[i*nat+j]=3.0+drand48();
      pc[i*nat+j]=5.0+drand48();
      pd[i*nat+j]=4.0+drand48();
      zz[i*nat+j]=-1.0+2.0*drand48();
    }
  }

  // timer start
  get_cputime(&ltime,&stime);

  // calc with target routine
  for(i=0;i<nstep;i++){
    switch(argv[2][0]){
    case '0':
      MR3calcnacl(x,n,atype,nat,pol,sigm,ipotro,pc,pd,zz,0,xmax,pflag,a2);
      if(i==0) printf("Original routine is used\n");
      break;
    case '1':
      MR3calcnacl_CPU(x,n,atype,nat,pol,sigm,ipotro,pc,pd,zz,0,xmax,pflag,a2);
      if(i==0) printf("CPU routine is used\n");
      break;
    case '2':
      MR3calcnacl_AVX(x,n,atype,nat,pol,sigm,ipotro,pc,pd,zz,0,xmax,pflag,a2);
      if(i==0) printf("AVX routine is used\n");
      break;
    case '3':
      MR3calcnacl_GPU(x,n,atype,nat,pol,sigm,ipotro,pc,pd,zz,0,xmax,pflag,a2);
      if(i==0) printf("GPU routine is used\n");
      break;
    default:
      fprintf(stderr,"** error : cal_mode=%c is not supported **\n",argv[2][0]);
      return 1;
    }
  }

  // timer stop
  get_cputime(&ltime,&stime);
  printf("Time=%f seconds, %f Gflops\n",
	 stime,((double)n)*((double)n)*((double)nstep)*78.0/stime/1e9);

  // check result
  MR3calcnacl(x,n,atype,nat,pol,sigm,ipotro,pc,pd,zz,0,xmax,pflag,a1);
  avr=0.0;
  for(i=0;i<n;i++){
    aone=0.0;
    for(j=0;j<3;j++){
      aone+=a1[i*3+j]*a1[i*3+j];
    }
    aone=sqrt(aone);
    avr+=aone;
  }
  avr/=(double)n;
  for(i=0;i<3;i++){
    printf("a1[%d]=%10.3e %e %10.3e a2=%10.3e %10.3e %10.3e\n",
	   i,a1[i*3+0],a1[i*3+1],a1[i*3+2],a2[i*3+0],a2[i*3+1],a2[i*3+2]);
  }
  printf("Average size of force is %10.3e\n",avr);
  err=0.0;
  for(i=0;i<n;i++){
    eone=0.0;
    for(j=0;j<3;j++){
      eone+=(a1[i*3+j]-a2[i*3+j])*(a1[i*3+j]-a2[i*3+j]);
    }
    eone=sqrt(eone)/avr;
    err+=eone;
  }
  err/=(double)n;
  printf("Average error is %10.3e\n",err);

  // deallocate variables
  free(x);
  free(a1);
  free(a2);
  free(atype);
  free(pol);
  free(sigm);
  free(ipotro);
  free(pc);
  free(pd);
  free(zz);
  
  return 0;
}
