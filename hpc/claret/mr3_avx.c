#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <x86intrin.h>
#include "fmath.hpp"
#define _mm256_exp_ps(x) fmath::exp_ps256(x)

#define MALLOC_AND_D2F(n,double_mem,float_mem) \
  posix_memalign((void **)&(float_mem),32,sizeof(float)*n);\
  {int i;for(i=0;i<(n);i++) (float_mem)[i]=(double_mem)[i];}

__m256 _mm256_exp_ps_alt(__m256 m256_x){

	__m256 m256_flags = _mm256_cmp_ps(m256_x, _mm256_set1_ps(-20.0f), _CMP_LT_OQ);
	m256_x = _mm256_blendv_ps(m256_x, _mm256_set1_ps(0.0), m256_flags);
	__m256 m256_result = _mm256_exp_ps(m256_x);
	m256_result = _mm256_blendv_ps(m256_result, _mm256_set1_ps(0.0), m256_flags);
	return m256_result;
}


void AVX_kernel(float x[], int n, int atype[], int nat,
		float pol[], float sigm[], float ipotro[],
		float pc[], float pd[], float zz[],
		int tblno, float xmax_org, int periodicflag,
		float force[])
{
  int i,j,k,t;
  //float dn2,r,inr,inr2,inr4,inr8,d3,dr[3],fi[3];
  //float pb=0.338e-19/(14.39*1.60219e-19),dphir; 

  __m256i m256_nat = _mm256_set1_epi32(nat);
  __m256 m256_dn2, m256_r, m256_inr, m256_inr2, m256_inr4, m256_inr8, m256_d3, m256_dr[3], m256_fi[3];
  __m256 m256_pb = _mm256_set1_ps(0.338e-19/(14.39*1.60219e-19));
  __m256 m256_dphir; 

  	//#pragma omp parallel for private(k,j,fi,dn2,dr,r,inr,inr2,inr4,inr8,t,d3,dphir)
  	#pragma omp parallel for private(k,j,m256_fi,m256_dr,m256_dn2, m256_r, m256_inr, m256_inr2, m256_inr4, m256_inr8, m256_d3, m256_dphir)
  	for(i=0; i<n/8; i++){
		int i8 = i*8;

   		//for(k=0; k<3; k++) fi[k] = 0.0f;
		for(k=0; k<3; k++) m256_fi[k] = _mm256_set1_ps(0.0);
		
    		for(j=0; j<n; j++){
			m256_dn2 = _mm256_set1_ps(0.0); //dn2 = 0.0f;

      			for(k=0; k<3; k++){
				__m256 m256_x_j = _mm256_set1_ps(x[j*3+k]);
				__m256 m256_x_i = _mm256_set_ps(x[(i8+7)*3+k], x[(i8+6)*3+k], x[(i8+5)*3+k], x[(i8+4)*3+k],
								x[(i8+3)*3+k], x[(i8+2)*3+k], x[(i8+1)*3+k], x[(i8+0)*3+k]);

				m256_dr[k] = _mm256_sub_ps(m256_x_i, m256_x_j);
				m256_dn2   = _mm256_fmadd_ps(m256_dr[k], m256_dr[k], m256_dn2);

				//dr[k] = x[i*3+k] - x[j*3+k];
				//dn2  += dr[k] * dr[k];
			}

			/*
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
			*/

			//r = sqrtf(dn2); 
			////inr   = 1.0f  / sqrt(dn2);
			m256_r    = _mm256_sqrt_ps(m256_dn2);			
			m256_inr  = _mm256_div_ps(_mm256_set1_ps(1.0f),m256_r); 	
			m256_inr2 = _mm256_mul_ps(m256_inr, m256_inr);	//inr2  = inr  * inr;
			m256_inr4 = _mm256_mul_ps(m256_inr2, m256_inr2); //inr4  = inr2 * inr2;
			m256_inr8 = _mm256_mul_ps(m256_inr4, m256_inr4); //inr8  = inr4 * inr4;


			__m256i m256_atype_j = _mm256_set1_epi32(atype[j]);
			__m256i m256_atype_i = _mm256_set_epi32(atype[i8+7], atype[i8+6], atype[i8+5], atype[i8+4],
								atype[i8+3], atype[i8+2], atype[i8+1], atype[i8+0]);
  			__m256i m256_t;
			//m256_t   = _mm256_mullo_epi32(m256_atype_i, m256_nat); //t = atype[i] * nat 
			m256_t   = _mm256_mullo_epi32(m256_atype_i, m256_nat); //t = atype[i] * nat 
			m256_t   = _mm256_add_epi32(m256_t, m256_atype_j);   //+ atype[j];
			

			__m256 m256_pol 	= _mm256_i32gather_ps(pol, m256_t, 4); //pol[t]
			__m256 m256_sigm 	= _mm256_i32gather_ps(sigm, m256_t, 4); //sigm[t]
			__m256 m256_ipotro 	= _mm256_i32gather_ps(ipotro, m256_t, 4); //ipotro[t]
			m256_d3 = _mm256_sub_ps(m256_sigm, m256_r);
			m256_d3 = _mm256_exp_ps_alt(_mm256_mul_ps(m256_ipotro, m256_d3));
			m256_d3 = _mm256_mul_ps(m256_pb, m256_d3);
			m256_d3 = _mm256_mul_ps(m256_pol, m256_d3);//d3    = pb * pol[t] * expf( (sigm[t] - r) * ipotro[t]);


			__m256 tmp0 = _mm256_mul_ps(m256_d3, _mm256_mul_ps(m256_ipotro, m256_inr));//d3 * ipotro[t] * inr
												   
			
			__m256 m256_pc = _mm256_i32gather_ps(pc, m256_t, 4); //pc[t]
			__m256 tmp1 = _mm256_mul_ps(_mm256_set1_ps(-6.0), _mm256_mul_ps(m256_pc, m256_inr8));//-6.0f * pc[t] * inr8
													    
			__m256 m256_pd = _mm256_i32gather_ps(pd, m256_t, 4); //pd[t]
			__m256 tmp2 = _mm256_mul_ps(_mm256_set1_ps(-8.0), _mm256_mul_ps(m256_pd, _mm256_mul_ps(m256_inr8, m256_inr2)));//-8.0f * pd[t] * inr8 * inr2
																      
			__m256 m256_zz = _mm256_i32gather_ps(zz, m256_t, 4); //zz[t]
			__m256 tmp3 = _mm256_mul_ps(m256_inr2, _mm256_mul_ps(m256_inr, m256_zz));//inr2 * inr * zz[t]
											    
			/*
			dphir = ( d3 * ipotro[t] * inr
				  - 6.0f * pc[t] * inr8
				  - 8.0f * pd[t] * inr8 * inr2
				  + inr2 * inr * zz[t] );
				  */
			m256_dphir = _mm256_add_ps(tmp0, tmp1);
			m256_dphir = _mm256_add_ps(m256_dphir, tmp2);
			m256_dphir = _mm256_add_ps(m256_dphir, tmp3);

			__m256 m256_flags = _mm256_cmp_ps(m256_dn2, _mm256_set1_ps(0.0), _CMP_EQ_OQ);
			m256_dphir = _mm256_blendv_ps(m256_dphir, _mm256_set1_ps(0.0), m256_flags);

			for(k=0; k<3; k++){
				//fi[k] += dphir * dr[k];
				m256_fi[k] = _mm256_fmadd_ps(m256_dphir, m256_dr[k], m256_fi[k]);
			}

	    	}

		float *fi_x_v = (float*)&m256_fi[0];
		float *fi_y_v = (float*)&m256_fi[1];
		float *fi_z_v = (float*)&m256_fi[2];

		for(k=0; k<8; k++){
			force[(i8+k)*3+0] = fi_x_v[k];
			force[(i8+k)*3+1] = fi_y_v[k];
			force[(i8+k)*3+2] = fi_z_v[k];
		}
  	}
}
    

extern "C"
void MR3calcnacl_AVX(double x[], int n, int atype[], int nat,
		     double pol[], double sigm[], double ipotro[],
		     double pc[], double pd[], double zz[],
		     int tblno, double xmax, int periodicflag,
		     double force[])
{
  int i,*f_atype;
  float *f_x,*f_pol,*f_sigm,*f_ipotro,*f_pc,*f_pd,*f_zz,*f_force,xmaxf=xmax;

#ifdef IGNORE_PERIODIC_FLAG
  if(periodicflag!=0){
    fprintf(stderr,"** error : periodicflag = %d is not supported **\n",periodicflag);
    exit(1);
  }
#endif
  if(n % 8!=0){
    fprintf(stderr,"** error : n must be multiple of 8 **\n");
    exit(1);
  }
  if(nat != 2){
    fprintf(stderr,"** error : nat must be 2 **\n");
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

  AVX_kernel(f_x,n,atype,nat,f_pol,f_sigm,f_ipotro,
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

