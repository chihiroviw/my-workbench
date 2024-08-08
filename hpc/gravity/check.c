#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#define EPS2    (0.03f*0.03f)

void float_force_optimized(int n, double pos[][4], double force[][4]);


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


void double_force(int n,                // input  : number of particles
		  double pos[][4],      // input  : position and mass of particles
		  double force[][4])    // output : force and potential of particles
{
  int i,j,k;
  double dr[3],r_1,dtmp,r2;

  for(i=0;i<n;i++){
    for(k=0;k<4;k++) force[i][k]=0.0;
    for(j=0;j<n;j++){
      r2=0.0;
      for(k=0;k<3;k++){
	dr[k]=pos[i][k]-pos[j][k];
	r2+=dr[k]*dr[k];
      }
      if(r2!=0.0){
	r_1=1.0/sqrt(r2+EPS2);
	dtmp=pos[j][3]*r_1;
	force[i][3]+=dtmp;
	dtmp*=r_1*r_1;
	for(k=0;k<3;k++) force[i][k]-=dtmp*dr[k];
      }
    }
    for(k=0;k<4;k++) force[i][k]*=pos[i][3];
  }
}


static void float_force_sub(int n,                // input  : number of particles
			    float posf[][4],      // input  : position and mass of particles
			    float forcef[][4])    // output : force and potential of particles
{
  int i,j,k;
  float dr[3],r_1,dtmp,r2;

  for(i=0;i<n;i++){
    for(k=0;k<4;k++) forcef[i][k]=0.0f;
    for(j=0;j<n;j++){
      r2=0.0f;
      for(k=0;k<3;k++){
	dr[k]=posf[i][k]-posf[j][k];
	r2+=dr[k]*dr[k];
      }
      if(r2!=0.0f){
	r_1=1.0f/sqrtf(r2+EPS2);
	dtmp=posf[j][3]*r_1;
	forcef[i][3]+=dtmp;
	dtmp*=r_1*r_1;
	for(k=0;k<3;k++) forcef[i][k]-=dtmp*dr[k];
      }
    }
    for(k=0;k<4;k++) forcef[i][k]*=posf[i][3];
  }
}


void float_force(int n,                // input  : number of particles
		 double pos[][4],      // input  : position and mass of particles
		 double force[][4])    // output : force and potential of particles
{
  int i,j,k;
  float (*posf)[4],(*forcef)[4];

  // Allocate and copy positions and masses
  posf=(float (*)[4])malloc(sizeof(float)*n*4);
  forcef=(float (*)[4])malloc(sizeof(float)*n*4);
  for(j=0;j<n;j++) for(k=0;k<4;k++) posf[j][k]=(float)pos[j][k];

  // Calculate force and potential with float
  float_force_sub(n,posf,forcef);

  // Copy back to double array
  for(i=0;i<n;i++) for(k=0;k<4;k++) force[i][k]=(double)forcef[i][k];

  free(posf);free(forcef);
}


void update_position(int n, double x[][4], double a[][4])
{
  int i,j;

  for(i=0;i<n;i++){
    for(j=0;j<3;j++){
      x[i][j]+=drand48()*0.01;
    }
  }
}


int main(int argc, char **argv)
{
  int i,j,n,nstep=1;
  double (*x)[4],(*a1)[4],(*a2)[4];
  double size=10.0,eps2=0.0;
  double ltime,stime;
  double avr,aone,err,eone;

  if(argc!=3 && argc!=4){
    printf("usage : %s number_of_particles calc_mode (number_of_steps)\n",argv[0]);
    printf("  calc_mode : 0 -- host double precision\n");
    printf("              1 -- host single precision (float)\n");
    printf("              2 -- host single precision optimized (float)\n");
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
  x=(double (*)[4])malloc(sizeof(double)*n*4);
  a1=(double (*)[4])malloc(sizeof(double)*n*4);
  a2=(double (*)[4])malloc(sizeof(double)*n*4);

  // set positions and masses
  for(i=0;i<n;i++){
    for(j=0;j<3;j++) x[i][j]=drand48()*size;
    x[i][3]=1.0;
  }

  // timer start
  get_cputime(&ltime,&stime);

  // calc with target routine
  for(i=0;i<nstep;i++){
    switch(argv[2][0]){
    case '0':
      double_force(n,x,a2);
      if(i==0) printf("double_force is called\n");
      break;
    case '1':
      float_force(n,x,a2);
      if(i==0) printf("float_force is called\n");
      break;
    case '2':
      float_force_optimized(n,x,a2);
      if(i==0) printf("float_force_optimized is called\n");
      break;
    default:
      fprintf(stderr,"** error : cal_mode=%c is not supported **\n",argv[2][0]);
      return 1;
    }
    if(i!=nstep-1) update_position(n,x,a2);
  }

  // timer stop
  get_cputime(&ltime,&stime);
  printf("Time=%f seconds, %f Gflops\n",
	 stime,((double)n)*((double)n)*((double)nstep)*38.0/stime/1e9);

  // check result
  double_force(n,x,a1);
  avr=0.0;
  for(i=0;i<n;i++){
    aone=0.0;
    for(j=0;j<3;j++){
      aone+=a1[i][j]*a1[i][j];
    }
    aone=sqrt(aone);
    avr+=aone;
  }
  avr/=(double)n;
  for(i=0;i<3;i++){
    printf("a1[%d]=%10.3e %e %10.3e a2=%10.3e %10.3e %10.3e\n",
	   i,a1[i][0],a1[i][1],a1[i][2],a2[i][0],a2[i][1],a2[i][2]);
  }
  printf("Average size of force is %10.3e\n",avr);
  err=0.0;
  for(i=0;i<n;i++){
    eone=0.0;
    for(j=0;j<3;j++){
      eone+=(a1[i][j]-a2[i][j])*(a1[i][j]-a2[i][j]);
    }
    eone=sqrt(eone)/avr;
    err+=eone;
  }
  err/=(double)n;
  printf("Average error of force is %10.3e\n",err);

  err=0.0;
  for(i=0;i<n;i++){
    err+=fabs((a1[i][3]-a2[i][3])/a1[i][3]);
  }
  err/=(double)n;
  printf("Average relative error of potential is %10.3e\n",err);

  // deallocate variables
  free(x);free(a1);free(a2);
  
  return 0;
}
