#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#define NMAX    8192
#define NTHRE   (64)
#define ATYPE        8
#define ATYPE2    (ATYPE * ATYPE)

typedef struct {
    float r[3];
    int atype;
} VG_XVEC;

typedef struct {
    float pol;
    float sigm;
    float ipotro;
    float pc;
    float pd;
    float zz;
} VG_MATRIX;

__constant__ VG_MATRIX d_matrix[sizeof(VG_MATRIX)*2*2];


__device__ __inline__ 
void inter(float xj[3], float xi[3], float fi[3], 
	        //int t, float xmax, float xmax1, VG_MATRIX* d_matrix){
	        int t, float xmax, float xmax1){

    int k;
    float dn2,r,inr,inr2,inr4,inr8,d3,dr[3];
    float pb=(float)(0.338e-19/(14.39*1.60219e-19)),dphir;

    dn2 = 0.0f;
    for(k=0; k<3; k++){
        dr[k]  = xi[k] - xj[k];
        dr[k] -= rintf(dr[k] * xmax1) * xmax;
        dn2   += dr[k] * dr[k];
    }

    //if(dn2 != 0.0f){
        r     = sqrtf(dn2);
        inr   = 1.0f / r;
        inr2  = inr  * inr;
        inr4  = inr2 * inr2;
        inr8  = inr4 * inr4;
        d3    = pb * d_matrix[t].pol * expf( (d_matrix[t].sigm - r) * d_matrix[t].ipotro);
        dphir = ( d3 * d_matrix[t].ipotro * inr
	            - 6.0f * d_matrix[t].pc * inr8
	            - 8.0f * d_matrix[t].pd * inr8 * inr2
	            + inr2 * inr * d_matrix[t].zz );
        //for(k=0; k<3; k++) fi[k] += dphir * dr[k];
    //}
    if(dn2 == 0.0f) dphir = 0; 
    for(k=0; k<3; k++) fi[k] += dphir * dr[k];
}

extern "C" __global__
void nacl_kernel_gpu(VG_XVEC *x, int n, int nat, float xmax, float *fvec){
//void nacl_kernel_gpu(VG_XVEC *x, int n, int nat, float xmax, float *fvec,VG_MATRIX* d_matrix){
//void nacl_kernel_gpu(float* r,int* atype, int n, int nat, float xmax, float *fvec,VG_MATRIX* d_matrix){
    int tid = threadIdx.x;
    int i = blockIdx.x * NTHRE + tid;
    int j,k;
    float fi[3],xmax1=1.0f/xmax;
    int atypei;
    float xi[3];

    
    for(k=0; k<3; k++) fi[k] = 0.0f;

    //for(int tt=0; tt<11; tt++){

    for(k=0; k<3; k++) xi[k] = x[i].r[k];
    atypei = x[i].atype * nat;

    //for(k=0; k<3; k++) xi[k] = r[i*3+k];
    //atypei = atype[i] * nat;
    //for (j = 0; j < n; j++){
    //    inter(x[j].r, xi, fi, atypei + x[j].atype, xmax, xmax1, d_matrix);
        //inter(&r[j*3], xi, fi, atypei + atype[j], xmax, xmax1, d_matrix);
    //}
    
    
    //sheared
    __shared__ VG_XVEC s_xj[NTHRE];
    for (j = 0; j < n; j+=64){

        //copy to shared memory
        __syncthreads();
        s_xj[threadIdx.x] = x[j+threadIdx.x]; 
        __syncthreads();

        for(int js=0; js<min(64,n-j); js++){
            //inter(s_xj[js].r, xi, fi, atypei + s_xj[js].atype, xmax, xmax1,d_matrix);
            inter(s_xj[js].r, xi, fi, atypei + s_xj[js].atype, xmax, xmax1);
        }
    }

    //}

    if(i<n) for(k=0; k<3; k++) fvec[i*3+k] = fi[k];;
}

extern "C"
void MR3calcnacl_GPU(double x[], int n, int atype[], int nat,
		     double pol[], double sigm[], double ipotro[],
		     double pc[], double pd[], double zz[],
		     int tblno, double xmax, int periodicflag,
		     double force[])
{

    int i,j;
    static VG_XVEC *d_x=NULL;
    static float *d_force=NULL; 


    //VG_MATRIX *d_matrix;
    //float *d_r;
    //int *d_atype;

    float xmaxf;
    VG_MATRIX *matrix=(VG_MATRIX *)force;
    static VG_XVEC   *vec=NULL;
    if((periodicflag & 1)==0) xmax*=2.0;
    xmaxf=xmax;
    static float *forcef=NULL;
    static int n_bak=0;
        
    // allocate global memory
    
    if(n!=n_bak){
    //if(true){
        // free and allocate global memory
        int nalloc;
        static int nalloc_bak=0;

        if(n>NMAX) nalloc=n;
        else       nalloc=NMAX;

        if(nalloc!=nalloc_bak){
        //if(true){
            (cudaFree(d_x));
            (cudaFree(d_force));
            (cudaFree(d_matrix));
            (cudaMalloc((void**)&d_x,sizeof(VG_XVEC)*(nalloc+NTHRE)));
            (cudaMalloc((void**)&d_force,sizeof(float)*nalloc*3));
            (cudaMalloc((void**)&d_matrix,sizeof(VG_MATRIX)*nat*nat));
            
            //(cudaFree(d_r));
            //(cudaFree(d_atype));
            //(cudaMalloc((void**)&d_r,sizeof(float)*3*nalloc));
            //(cudaMalloc((void**)&d_atype,sizeof(int)*nalloc));

            free(vec);
            vec=(VG_XVEC *)malloc(sizeof(VG_XVEC)*(nalloc+NTHRE));
            free(forcef);
            forcef=(float *)malloc(sizeof(float)*nalloc*3);
            bzero(forcef,sizeof(float)*nalloc*3);
            nalloc_bak=nalloc;
        }
        // set matrix
        for(i=0;i<nat;i++){
            for(j=0;j<nat;j++){
	            matrix[i*nat+j].pol=(float)(pol[i*nat+j]);
	            matrix[i*nat+j].sigm=(float)(sigm[i*nat+j]);
                matrix[i*nat+j].ipotro=(float)(ipotro[i*nat+j]);
                matrix[i*nat+j].pc=(float)(pc[i*nat+j]);
                matrix[i*nat+j].pd=(float)(pd[i*nat+j]);
                matrix[i*nat+j].zz=(float)(zz[i*nat+j]);
            }
        }

        //constant memory
        (cudaMemcpyToSymbol(d_matrix,matrix,sizeof(VG_MATRIX)*nat*nat));
        //(cudaMemcpy(d_matrix,matrix,sizeof(VG_MATRIX)*nat*nat,cudaMemcpyHostToDevice));

        n_bak=n;
    }
    
    // copy from host to GPU
    for(i=0;i<(n+NTHRE-1)/NTHRE*NTHRE;i++){
        if(i<n){
            for(j=0;j<3;j++){
	            vec[i].r[j]=x[i*3+j];
            }
            vec[i].atype=atype[i];

        }else{
            for(j=0;j<3;j++){
	            vec[i].r[j]=0.0f;
            }
            vec[i].atype=0;
        }
    }
    
    (cudaMemcpy(d_x,vec,sizeof(VG_XVEC)*((n+NTHRE-1)/NTHRE*NTHRE),cudaMemcpyHostToDevice));
    
    /*
    float *r_vec = (float*)malloc(sizeof(float)*3*n);
    for(int i=0; i<n; i++){
        r_vec[i*3+0] = x[i*3+0];
        r_vec[i*3+1] = x[i*3+1];
        r_vec[i*3+2] = x[i*3+2];
    }
    (cudaMemcpy(d_r,r_vec,sizeof(float)*3*n,cudaMemcpyHostToDevice));
    (cudaMemcpy(d_atype,atype,sizeof(int)*n,cudaMemcpyHostToDevice));
    */
    

    // call GPU kernel
    dim3 threads(NTHRE);
    dim3 grid((n+NTHRE-1)/NTHRE);
    nacl_kernel_gpu<<< grid, threads >>>(d_x,n,nat,xmaxf,d_force);
    //nacl_kernel_gpu<<< grid, threads >>>(d_x,n,nat,xmaxf,d_force,d_matrix);
    //nacl_kernel_gpu<<< grid, threads >>>(d_r,d_atype,n,nat,xmaxf,d_force,d_matrix);

    // copy GPU result to host
    (cudaMemcpy(forcef,d_force,sizeof(float)*n*3,cudaMemcpyDeviceToHost));
    for(i=0;i<n;i++){ 
        for(j=0;j<3;j++){ 
            force[i*3+j]=forcef[i*3+j];
        }
    }
}
