#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <float.h>
#include <string.h>

#ifdef _WIN32
#define MYGLDEF_1 WINGDIAPI
#define MYGLDEF_2 APIENTRY
#else
#define MYGLDEF_1 GLAPI
#define MYGLDEF_2 GLAPIENTRY
#endif

static int WIDTH=1, HEIGHT=1;
static float (**Zb)=NULL;
static float *Zb_all;
static unsigned char (**Fb)[3]=NULL;
static unsigned char (*Fb_all)[3];
static unsigned char (**Tx)[4]=NULL;
static unsigned char (*Tx_all)[4];
static int TEX_HEIGHT = 1, TEX_WIDTH = 1;
static unsigned char ClearColor[4]={0,0,0,255};
static unsigned char CurrentColor[4]={255,255,255,255};
static float         CurrentTexCoord[2]={0.0,0.0};
static float         CurrentNormal[3]={0.0,0.0,0.0};
static int Glbegin_flag=0;
static GLenum Glbegin_mode;
static float Last_pos[3], Last_pos2[3], Last_pos3[3];
static float Last_wpos[3], Last_wpos2[3], Last_wpos3[3];
static unsigned char Last_color[3], Last_color2[3], Last_color3[3];
static float Last_texcoord[2], Last_texcoord2[2], Last_texcoord3[2];
static float Last_normal[3], Last_normal2[3], Last_normal3[3];
static int GlEnable_cap=0;
enum capability_bit {MYGL_TEXTURE, MYGL_BLEND, MYGL_DEPTH_TEST, MYGL_CULL_FACE,
		     MYGL_ALPHA_TEST, MYGL_LIGHTING, MYGL_LIGHT0,
		     num_capability};
static float AlphaRef=0.0;
static float Modelmatrix[4][4]={{1.0,0.0,0.0,0.0},
				{0.0,1.0,0.0,0.0},
				{0.0,0.0,1.0,0.0},
				{0.0,0.0,0.0,1.0}};
static float Projectmatrix[4][4]={{1.0,0.0,0.0,0.0},
				  {0.0,1.0,0.0,0.0},
				  {0.0,0.0,1.0,0.0},
				  {0.0,0.0,0.0,1.0}};
static float (*Currentmatrix)[4] = Modelmatrix;
#define MYGL_STACK_SIZE 1000
static float Stackmatrix[MYGL_STACK_SIZE][4][4];
static int Stackindex=0;
static float Light0_pos[4]={0.0,0.0,1.0,0.0};
static float Light0_ambient[4]={0.0,0.0,0.0,1.0};
static float Light0_la_ambient[4]={0.0,0.0,0.0,1.0};
static float Light0_diffuse[4]={1.0,1.0,1.0,1.0};
static float Light0_specular[4]={1.0,1.0,1.0,1.0};
static int LightModel_localviewer=GL_FALSE;
static float Material_ambient[4]={0.0,0.0,0.0,1.0};
static float Material_diffuse[4]={1.0,1.0,1.0,1.0};
static float Material_specular[4]={0.0,0.0,0.0,1.0};
static float Material_shininess=1.0;

#define V2WX(x) (((x)+1.0)*WIDTH/2.0-0.5)
#define V2WY(y) (((y)+1.0)*HEIGHT/2.0-0.5)

static float dot(GLfloat a[3], GLfloat b[3])
{
  float s = 0.0f;
  int i;
  for(i=0;i<3;i++) s += a[i] * b[i];
  return s;
}

static void cross(GLfloat a[3], GLfloat b[3], GLfloat c[3])
{
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

static float vecsize(GLfloat a[3])
{
  GLfloat size = 0.0f;
  int i;
  for(i=0;i<3;i++) size += a[i] * a[i];
  size = sqrtf(size);
  return size;
}

static void normalize(GLfloat a[3])
{
  GLfloat size = vecsize(a);
  int i;
  for(i=0;i<3;i++) a[i] /= size;
}

static void clampcolor(int ci[3], unsigned char co[3])
{
  int i;
  for(i=0;i<3;i++){
    if(ci[i]<0)        co[i]=0;
    else if(ci[i]>255) co[i]=255;
    else               co[i]=ci[i];
  }
}

static void matrix_vec_3f(float A[3][3], float B[], float C[])
{
  int i,j;
  for(i=0; i<3; i++){
    C[i]=0.0f;
    for(j=0; j<3; j++){
      C[i]+=A[j][i]*B[j];
    }
  }
}

static void matrix_vec_43f(float A[4][4], float B[], float C[])
{
  int i,j;
  float out[3];
  for(i=0; i<3; i++){
    out[i]=0.0f;
    for(j=0; j<3; j++){
      out[i]+=A[j][i]*B[j];
    }
  }
  for(i=0; i<3; i++) C[i]=out[i];
}

static void matrix_vec_4f(float A[4][4], float B[4], float C[4])
{
  int i,j;
  float out[4];
  for(i=0; i<4; i++){
    out[i]=0.0f;
    for(j=0; j<4; j++){
      out[i]+=A[j][i]*B[j];
    }
  }
  for(i=0; i<4; i++) C[i]=out[i];
}

static void matrix_mul_34f(float A[3][3], float B[4][4], float C[3][3])
{
  int i,j,k;
  float out[3][3];
  for(i=0; i<3; i++){
    for(j=0; j<3; j++){
      out[j][i] = 0.0f;
      for(k=0; k<3; k++){
	out[j][i] += A[k][i] * B[j][k];
      }
    }
  }
  for(i=0; i<3; i++){
    for(j=0; j<3; j++){
      C[j][i] = out[j][i];
    }
  }
}

static void matrix_mul_4f(float A[4][4], float B[4][4], float C[4][4])
{
  int i,j,k;
  float out[4][4];
  for(i=0; i<4; i++){
    for(j=0; j<4; j++){
      out[j][i] = 0.0f;
      for(k=0; k<4; k++){
	out[j][i] += A[k][i] * B[j][k];
      }
    }
  }
  for(i=0; i<4; i++){
    for(j=0; j<4; j++){
      C[j][i] = out[j][i];
    }
  }
}

static void drawpointC(int x, int y, unsigned char *cl)
{
  int c;
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT){
    for(c=0;c<3;c++) Fb[y][x][c]=cl[c];
  }
}

void drawpointCZ(int x, int y, float z, unsigned char *cl)
{
  int c;
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT && z>=-1.0f && z<=1.0f){
    if(z<Zb[y][x]){
      for(c=0;c<3;c++) Fb[y][x][c]=cl[c];
      Zb[y][x] = z;
    }
  }
}

static void drawpoint(int x, int y)
{
  drawpointC(x,y,CurrentColor);
}

static int drawtriangle_sub(int x, int y, float x0, float y0, float x1, float y1)
{
  /* 0 -- right hand side of line (x0,y0)-(x1,y1)
     1 -- left hand side of line (x0,y0)-(x1,y1)
  */
  if((x1-x0)*(y-y0)-(y1-y0)*(x-x0)>=0.0f) return 1;
  else                                    return 0;
}

static void drawtriangle_calcminmax(float x0,  float y0,
				    float x1, float y1,
				    float x2, float y2,
				    int minmax[2][2])
{
  if(x0<=x1 && x0<=x2)      minmax[0][0]=floorf(x0);
  else if(x1<=x0 && x1<=x2) minmax[0][0]=floorf(x1);
  else                      minmax[0][0]=floorf(x2);
  if(y0<=y1 && y0<=y2)      minmax[0][1]=floorf(y0);
  else if(y1<=y0 && y1<=y2) minmax[0][1]=floorf(y1);
  else                      minmax[0][1]=floorf(y2);
  if(x0>=x1 && x0>=x2)      minmax[1][0]=ceilf(x0);
  else if(x1>=x0 && x1>=x2) minmax[1][0]=ceilf(x1);
  else                      minmax[1][0]=ceilf(x2);
  if(y0>=y1 && y0>=y2)      minmax[1][1]=ceilf(y0);
  else if(y1>=y0 && y1>=y2) minmax[1][1]=ceilf(y1);
  else                      minmax[1][1]=ceilf(y2);

  if(minmax[0][0]<0) minmax[0][0]=0;
  if(minmax[0][1]<0) minmax[0][1]=0;
  if(minmax[1][0]>=WIDTH)  minmax[1][0]=WIDTH-1;
  if(minmax[1][1]>=HEIGHT) minmax[1][1]=HEIGHT-1;
}

static void drawtriangle(float x0, float y0, float x1, float y1, float x2, float y2)
{
  int minmax[2][2],x,y;

  drawtriangle_calcminmax(x0, y0, x1, y1, x2, y2, minmax);
  for(y=minmax[0][1]; y<= minmax[1][1]; y++){
    for(x=minmax[0][0]; x<= minmax[1][0]; x++){
      if(drawtriangle_sub(x,y,x0,y0,x1,y1) &&
	 drawtriangle_sub(x,y,x1,y1,x2,y2) &&
	 drawtriangle_sub(x,y,x2,y2,x0,y0)){
	drawpoint(x,y);
      }
    }
  }
}

static void drawtriangleC(float x0, float y0, float x1, float y1, float x2, float y2,
			  unsigned char c0[], unsigned char c1[], unsigned char c2[])
{
  int minmax[2][2],x,y;
  float u,v;
  int c,ci[3];
  unsigned char cl[3];

  drawtriangle_calcminmax(x0, y0, x1, y1, x2, y2, minmax);
  for(y=minmax[0][1]; y<= minmax[1][1]; y++){
    for(x=minmax[0][0]; x<= minmax[1][0]; x++){
      if(drawtriangle_sub(x,y,x0,y0,x1,y1) &&
	 drawtriangle_sub(x,y,x1,y1,x2,y2) &&
	 drawtriangle_sub(x,y,x2,y2,x0,y0)){
	u = ( (x-x0)*(y2-y0) - (x2-x0)*(y-y0) ) /
	  ( (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0) );
	v = ( (x-x0)*(y1-y0) - (x1-x0)*(y-y0) ) /
	  ( (x2-x0)*(y1-y0) - (x1-x0)*(y2-y0) );
	for(c=0; c<3; c++){
	  ci[c] = (c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]) );
	  if(ci[c]<0)         ci[c]=0;
	  else if(ci[c]>=256) ci[c]=255;
	  cl[c] = ci[c];
	}
	drawpointC(x,y,cl);
      }
    }
  }
}

static void drawtriangleT(float x0, float y0, float x1, float y1, float x2, float y2,
			  float tx0, float ty0, float tx1, float ty1, float tx2, float ty2)
{
  int minmax[2][2],x,y,c;
  float u,v;
  int tx,ty;
  unsigned char cl[3];
  
  drawtriangle_calcminmax(x0, y0, x1, y1, x2, y2, minmax);
  for(y=minmax[0][1]; y<= minmax[1][1]; y++){
    for(x=minmax[0][0]; x<= minmax[1][0]; x++){
      if(drawtriangle_sub(x,y,x0,y0,x1,y1) &&
	 drawtriangle_sub(x,y,x1,y1,x2,y2) &&
	 drawtriangle_sub(x,y,x2,y2,x0,y0)){
	u = ( (x-x0)*(y2-y0) - (x2-x0)*(y-y0) ) /
	  ( (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0) );
	v = ( (x-x0)*(y1-y0) - (x1-x0)*(y-y0) ) /
	  ( (x2-x0)*(y1-y0) - (x1-x0)*(y2-y0) );
	tx = (tx0 + u*(tx1-tx0) + v*(tx2-tx0) ) * TEX_WIDTH;// + 0.5f;
	if(tx<0)               tx=0;
	else if(tx>=TEX_WIDTH) tx=TEX_WIDTH-1;
	ty = (ty0 + u*(ty1-ty0) + v*(ty2-ty0) ) * TEX_HEIGHT;// + 0.5f;
	if(ty<0)                ty=0;
	else if(ty>=TEX_HEIGHT) ty=TEX_HEIGHT-1;
	if(GlEnable_cap & (1<<MYGL_BLEND)){ // blend
	  for(c=0; c<3; c++){
	    cl[c] = (Fb[y][x][c] * (255 - Tx[ty][tx][3]) +
		     Tx[ty][tx][c] * Tx[ty][tx][3]) >> 8;
	  }
	} else {                     // no blend
	  for(c=0; c<3; c++) cl[c]=Tx[ty][tx][c];
	}
	drawpointC(x,y,cl);
      }
    }
  }
}

static void drawtriangleCT(float x0, float y0, float x1, float y1, float x2, float y2,
			   unsigned char c0[], unsigned char c1[], unsigned char c2[],
			   float tx0, float ty0, float tx1, float ty1, float tx2, float ty2)
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of drawtriangleCT **\n");
    ini=1;
  }

    int minmax[2][2],x,y,c;
    float u,v;
    int tx,ty;
    unsigned char cl[3], cb[3];
  
    drawtriangle_calcminmax(x0,y0,x1,y1,x2,y2,minmax);

    for(y=minmax[0][1]; y<=minmax[1][1]; y++){
        for(x=minmax[0][0]; x<=minmax[1][0]; x++){
            if( drawtriangle_sub(x,y,x0,y0,x1,y1) &&
	            drawtriangle_sub(x,y,x1,y1,x2,y2) &&
	            drawtriangle_sub(x,y,x2,y2,x0,y0)){
                    u = ( (x-x0)*(y2-y0) - (x2-x0)*(y-y0) ) /
	                    ( (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0) );
	                v = ( (x-x0)*(y1-y0) - (x1-x0)*(y-y0) ) /
	                    ( (x2-x0)*(y1-y0) - (x1-x0)*(y2-y0) );
	                tx = (tx0 + u*(tx1-tx0) + v*(tx2-tx0) ) * TEX_WIDTH;// + 0.5f;
	                if(tx<0) tx=0;
                    else if(tx>=TEX_WIDTH) tx=TEX_WIDTH-1;
	                ty = (ty0 + u*(ty1-ty0) + v*(ty2-ty0) ) * TEX_HEIGHT;// + 0.5f;
	                if(ty<0) ty=0;
	                else if(ty>=TEX_HEIGHT) ty=TEX_HEIGHT-1;

                    for(c=0; c<3; c++){
                        cb[c]=c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]);
                    }
                    
                    for(c=0; c<3; c++){
                        cb[c]=(Tx[ty][tx][c]*cb[c])>>8; 
                    }

                    if(GlEnable_cap & (1<<MYGL_BLEND)){ // glEnable(GL_BLEND) is called 
                        for(c=0; c<3; c++){
	                        cl[c] = (Fb[y][x][c]*(255-Tx[ty][tx][3]) +
		                            cb[c]*Tx[ty][tx][3])>>8;
	                    }
                    } else {                            // glEnable(GL_BLEND) is not called 
                        for(c=0; c<3; c++) cl[c]=cb[c];
                    }
                    drawpointC(x,y,cl);
            }
        }
    } 
}

void drawtriangleCZ(float p0[], float p1[], float p2[],
		    unsigned char c0[], unsigned char c1[], unsigned char c2[])
{
  int minmax[2][2],x,y;
  float u,v,z;
  int c, ci[3];
  unsigned char cl[3];

  drawtriangle_calcminmax(p0[0], p0[1], p1[0], p1[1], p2[0], p2[1], minmax);
  for(y=minmax[0][1]; y<= minmax[1][1]; y++){
    for(x=minmax[0][0]; x<= minmax[1][0]; x++){
      if(drawtriangle_sub(x,y,p0[0],p0[1],p1[0],p1[1]) &&
	 drawtriangle_sub(x,y,p1[0],p1[1],p2[0],p2[1]) &&
	 drawtriangle_sub(x,y,p2[0],p2[1],p0[0],p0[1])){
	u = ( (x-p0[0])*(p2[1]-p0[1]) - (p2[0]-p0[0])*(y-p0[1]) ) /
	  ( (p1[0]-p0[0])*(p2[1]-p0[1]) - (p2[0]-p0[0])*(p1[1]-p0[1]) );
	v = ( (x-p0[0])*(p1[1]-p0[1]) - (p1[0]-p0[0])*(y-p0[1]) ) /
	  ( (p2[0]-p0[0])*(p1[1]-p0[1]) - (p1[0]-p0[0])*(p2[1]-p0[1]) );
	for(c=0; c<3; c++) ci[c] = (c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]) );
	clampcolor(ci,cl);
	z = p0[2] + u*(p1[2]-p0[2]) + v*(p2[2]-p0[2]);
	drawpointCZ(x,y,z,cl);
      }
    }
  }
}

void drawtriangleNZ_sub(unsigned char *c,float *p, float *wp, float *n){
    float L[3] = {Light0_pos[0]-wp[0],Light0_pos[1]-wp[1],Light0_pos[2]-wp[2]};
    normalize(L);

    float N[3]={n[0],n[1],n[2]};
    normalize(N);

    float NL = dot(N,L);
    NL = (0<NL)?NL:0;

    float V[3] = {-wp[0],-wp[1],-wp[2]};
    normalize(V);

    float H[3] = {L[0]+V[0],L[1]+V[1],L[2]+V[2]};
    normalize(H);

    float NH = dot(N,H);
    NH = (0<NH)?NH:0;
    int i;
    for(i=0; i<3; i++){
        float Ia = Material_ambient[i]*Light0_ambient[i];
        float Id = NL*Material_diffuse[i]*Light0_diffuse[i];
        float Is = pow(NH,Material_shininess)*Material_specular[i]*Light0_specular[i];
        c[i] = (Ia+Id+Is)*255;
    }
}

void drawtriangleNZ(float p0[], float p1[], float p2[],   // projected posotion
		    float wp0[], float wp1[], float wp2[],// world posotion
		    float n0[], float n1[], float n2[])   // normal vector
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of drawtriangleNZ **\n");
    ini=1;
  }

    unsigned char c0[4],c1[4],c2[4];
    c0[3] = c1[3] = c2[3] = 1;
    drawtriangleNZ_sub(c0, p0, wp0, n0);
    drawtriangleNZ_sub(c1, p1, wp1, n1);
    drawtriangleNZ_sub(c2, p2, wp2, n2);
    
    drawtriangleCZ(p0,p1,p2,c0,c1,c2);
}

void drawtriangleCTZ(float p0[], float p1[], float p2[],
		     unsigned char c0[], unsigned char c1[], unsigned char c2[],
		     float t0[], float t1[], float t2[])
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of drawtriangleCTZ **\n");
    ini=1;
  }
    
    int minmax[2][2],x,y,c;
    float z;
    float u,v;
    float tx0=t0[0],ty0=t0[1],tx1=t1[0],ty1=t1[1],tx2=t2[0],ty2=t2[1];
    float x0=p0[0],y0=p0[1],x1=p1[0],y1=p1[1],x2=p2[0],y2=p2[1];
    int tx,ty;
    unsigned char cl[3], cb[3];
  
    drawtriangle_calcminmax(x0,y0,x1,y1,x2,y2,minmax);

    for(y=minmax[0][1]; y<=minmax[1][1]; y++){
        for(x=minmax[0][0]; x<=minmax[1][0]; x++){
            if( drawtriangle_sub(x,y,x0,y0,x1,y1) &&
	            drawtriangle_sub(x,y,x1,y1,x2,y2) &&
	            drawtriangle_sub(x,y,x2,y2,x0,y0)){
                    u = ( (x-x0)*(y2-y0) - (x2-x0)*(y-y0) ) /
	                    ( (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0) );
	                v = ( (x-x0)*(y1-y0) - (x1-x0)*(y-y0) ) /
	                    ( (x2-x0)*(y1-y0) - (x1-x0)*(y2-y0) );
	                tx = (tx0 + u*(tx1-tx0) + v*(tx2-tx0) ) * TEX_WIDTH;// + 0.5f;
	                if(tx<0) tx=0;
                    else if(tx>=TEX_WIDTH) tx=TEX_WIDTH-1;
	                ty = (ty0 + u*(ty1-ty0) + v*(ty2-ty0) ) * TEX_HEIGHT;// + 0.5f;
	                if(ty<0) ty=0;
	                else if(ty>=TEX_HEIGHT) ty=TEX_HEIGHT-1;
                    
                    for(c=0; c<3; c++){
                        cb[c]=c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]);
                    }

                    for(c=0; c<3; c++){
                        cb[c]=(Tx[ty][tx][c]*cb[c])>>8; 
                    }
                    
                    if(GlEnable_cap & (1<<MYGL_ALPHA_TEST)){
                        if(AlphaRef<=Tx[ty][tx][3]){
                            for(c=0; c<3; c++)cl[c] = cb[c];
                        }else{
                            continue;
                        } 
                    }else if(GlEnable_cap & (1<<MYGL_BLEND)){ // glEnable(GL_BLEND) is called 
                        for(c=0; c<3; c++){
	                        cl[c] = (Fb[y][x][c]*(255-Tx[ty][tx][3]) +
		                            cb[c]*Tx[ty][tx][3])>>8;
	                    }
                    }else{                            // glEnable(GL_BLEND) is not called 
                        for(c=0; c<3; c++) cl[c]=cb[c];
                    }
                    
                    z=p0[2]+u*(p1[2]-p0[2])+v*(p2[2]-p0[2]);
                    drawpointCZ(x,y,z,cl);
            }
        }
    } 
}

void drawtriangleNTZ(float p0[], float p1[], float p2[],
		     float wp0[], float wp1[], float wp2[],// world posotion
		     float n0[], float n1[], float n2[],
		     float t0[], float t1[], float t2[])
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of drawtriangleNTZ **\n");
    ini=1;
  }
    unsigned char c0[4],c1[4],c2[4];
    c0[3] = c1[3] = c2[3] = 1;
    drawtriangleNZ_sub(c0, p0, wp0, n0);
    drawtriangleNZ_sub(c1, p1, wp1, n1);
    drawtriangleNZ_sub(c2, p2, wp2, n2);
    drawtriangleCTZ(p0,p1,p2,c0,c1,c2,t0,t1,t2);
 
}

MYGLDEF_1 void MYGLDEF_2 glClear( GLbitfield mask )
{
  int x,y,c;
  if(mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)){
    fprintf(stderr,"** error : not supported mask in glClear **\n");
    exit(1);
  }
  if(mask & GL_DEPTH_BUFFER_BIT){
    for(y=0; y<HEIGHT; y++){
      for(x=0; x<WIDTH; x++){
	Zb[y][x] = FLT_MAX;
      }
    }
  }
  if(mask & GL_COLOR_BUFFER_BIT){
    for(y=0; y<HEIGHT; y++){
      for(x=0; x<WIDTH; x++){
	for(c=0; c<3; c++){
	  Fb[y][x][c] = ClearColor[c];
	}
      }
    }
  }
}

MYGLDEF_1 void MYGLDEF_2 glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
  ClearColor[0] = 255 * red;
  ClearColor[1] = 255 * green;
  ClearColor[2] = 255 * blue;
  ClearColor[3] = 255 * alpha;
}

MYGLDEF_1 void MYGLDEF_2 glColor3ub( GLubyte red, GLubyte green, GLubyte blue )
{
  CurrentColor[0] = red;
  CurrentColor[1] = green;
  CurrentColor[2] = blue;
}

MYGLDEF_1 void MYGLDEF_2 glColor3f( GLfloat red, GLfloat green, GLfloat blue )
{
  CurrentColor[0] = 255 * red;
  CurrentColor[1] = 255 * green;
  CurrentColor[2] = 255 * blue;
}

MYGLDEF_1 void MYGLDEF_2 glColor3fv( const GLfloat *v )
{
  CurrentColor[0] = 255 * v[0];
  CurrentColor[1] = 255 * v[1];
  CurrentColor[2] = 255 * v[2];
}

MYGLDEF_1 void MYGLDEF_2 glBegin( GLenum mode )
{
  if(mode != GL_TRIANGLES){
    fprintf(stderr,"** error : mode is not supported in glBegin **\n");
    exit(1); 
  }
  Glbegin_mode = mode;
  Glbegin_flag = 1;
}

static void model_mat_3f(float in[3], float out[3])
{
  int i,j;
  float in4[4], out4[4];
  for(i=0; i<3; i++) in4[i] = in[i];
  in4[3] = 1.0f;
  matrix_vec_4f(Modelmatrix,in4,out4);
  for(i=0; i<3; i++) out[i] = out4[i]/out4[3];
}

static void model_proj_mat_3f(float in[3], float out[3])
{
  int i,j;
  float in4[4], out4[4];
  float mat[4][4];
  matrix_mul_4f(Projectmatrix,Modelmatrix,mat);
  for(i=0; i<3; i++) in4[i] = in[i];
  in4[3] = 1.0f;
  for(i=0; i<4; i++){
    out4[i] = 0.0f;
    for(j=0; j<4; j++){
      out4[i] += mat[j][i] * in4[j];
    }
  }
  for(i=0; i<3; i++) out[i] = out4[i]/out4[3];
}

static void myglVertex3f(float x, float y, float z)
{
  int c;
  
  for(c=0; c<3; c++){
    Last_pos3[c] = Last_pos2[c];
    Last_pos2[c] = Last_pos[c];
    Last_wpos3[c] = Last_wpos2[c];
    Last_wpos2[c] = Last_wpos[c];
  }
  float p[3]={x,y,z};
  model_proj_mat_3f(p,Last_pos);
  model_mat_3f(p,Last_wpos);

    ///////////////////////
    float cn[3] = { p[0]+CurrentNormal[0]
                    ,p[1]+CurrentNormal[1]
                    ,p[2]+CurrentNormal[2]};
    model_mat_3f(cn,CurrentNormal);
    int i;
    for(i=0; i<3; i++) CurrentNormal[i] -= Last_wpos[i];
    normalize(cn);
    ///////////////////////

  for(c=0; c<3; c++){
    Last_color3[c] = Last_color2[c];
    Last_color2[c] = Last_color[c];
    Last_color[c] = CurrentColor[c];
    Last_normal3[c] = Last_normal2[c];
    Last_normal2[c] = Last_normal[c];
    Last_normal[c] = CurrentNormal[c];
  }
  for(c=0; c<2; c++){
    Last_texcoord3[c] = Last_texcoord2[c];
    Last_texcoord2[c] = Last_texcoord[c];
    Last_texcoord[c] = CurrentTexCoord[c];
  }

  if(Glbegin_mode == GL_TRIANGLES &&
     Glbegin_flag % 3==0){
    if(GlEnable_cap & (1<<MYGL_TEXTURE)){
      if(GlEnable_cap & (1<<MYGL_DEPTH_TEST)){
	float p0[3]={V2WX(Last_pos3[0]), V2WY(Last_pos3[1]),Last_pos3[2]};
	float p1[3]={V2WX(Last_pos2[0]), V2WY(Last_pos2[1]),Last_pos2[2]};
	float p2[3]={V2WX(Last_pos[0]), V2WY(Last_pos[1]),Last_pos[2]};
	if(GlEnable_cap & (1<<MYGL_LIGHTING)){
	  drawtriangleNTZ(p0, p1, p2, Last_wpos3, Last_wpos2, Last_wpos,
			  Last_normal3, Last_normal2, Last_normal,
			  Last_texcoord3, Last_texcoord2, Last_texcoord);
	} else {
	  drawtriangleCTZ(p0, p1, p2, Last_color3, Last_color2, Last_color,
			  Last_texcoord3, Last_texcoord2, Last_texcoord);
	}
      } else {
	drawtriangleCT(V2WX(Last_pos3[0]), V2WY(Last_pos3[1]),
		       V2WX(Last_pos2[0]), V2WY(Last_pos2[1]),
		       V2WX(Last_pos[0]), V2WY(Last_pos[1]),
		       Last_color3, Last_color2, Last_color,
		       Last_texcoord3[0], Last_texcoord3[1],
		       Last_texcoord2[0], Last_texcoord2[1],
		       Last_texcoord[0], Last_texcoord[1]);
      }
    } else {
      if(GlEnable_cap & (1<<MYGL_DEPTH_TEST)){
	float p0[3]={V2WX(Last_pos3[0]), V2WY(Last_pos3[1]),Last_pos3[2]};
	float p1[3]={V2WX(Last_pos2[0]), V2WY(Last_pos2[1]),Last_pos2[2]};
	float p2[3]={V2WX(Last_pos[0]), V2WY(Last_pos[1]),Last_pos[2]};
	if(GlEnable_cap & (1<<MYGL_LIGHTING)){
	  drawtriangleNZ(p0, p1, p2, 
			 Last_wpos3, Last_wpos2, Last_wpos,
			 Last_normal3, Last_normal2, Last_normal);
	} else {
	  drawtriangleCZ(p0, p1, p2, Last_color3, Last_color2, Last_color);
	}
      } else {
	drawtriangleC(V2WX(Last_pos3[0]), V2WY(Last_pos3[1]),
		      V2WX(Last_pos2[0]), V2WY(Last_pos2[1]),
		      V2WX(Last_pos[0]), V2WY(Last_pos[1]),
		      Last_color3, Last_color2, Last_color);
      }
    }
  }
  Glbegin_flag ++;
}

MYGLDEF_1 void MYGLDEF_2 glVertex2f( GLfloat x, GLfloat y )
{
  myglVertex3f(x,y,0.0f);
}

MYGLDEF_1 void MYGLDEF_2 glVertex3f( GLfloat x, GLfloat y, GLfloat z )
{
  myglVertex3f(x,y,z);
}

MYGLDEF_1 void MYGLDEF_2 glVertex3fv( const GLfloat *v )
{
  myglVertex3f(v[0], v[1], v[2]);
}

MYGLDEF_1 void MYGLDEF_2 glEnd( void )
{
  Glbegin_mode = 0;
  Glbegin_flag = 0;
}

MYGLDEF_1 void MYGLDEF_2 glPixelStorei( GLenum pname, GLint param )
{
  if(pname != GL_UNPACK_ALIGNMENT || param != 4){
    fprintf(stderr,"** error : pname and param not supported in glPixelStorei **\n");
    exit(1);
  }
}

MYGLDEF_1 void MYGLDEF_2 glTexImage2D( GLenum target, GLint level,
				       GLint internalFormat,
				       GLsizei width, GLsizei height,
				       GLint border, GLenum format, GLenum type,
				       const GLvoid *pixels )
{
  int x,y,c;
  if(target != GL_TEXTURE_2D || level != 0 || internalFormat != GL_RGBA ||
     border != 0 || format != GL_RGBA || type != GL_UNSIGNED_BYTE){
    fprintf(stderr,"** error : parameters are not supported in glTexImage2D **\n");
    exit(1);
  }
  if(Tx != NULL){
    free(Tx);
    free(Tx_all);
  }
  if((Tx_all=(unsigned char (*)[4])malloc(sizeof(unsigned char)*4*width*height))==NULL){
    fprintf(stderr,"** can't malloc Tx_all in glTexImage2D **\n");
    exit(1);
  }
  if((Tx=(unsigned char (**)[4])malloc(sizeof(unsigned char (*)[4])*height))==NULL){
    fprintf(stderr,"** can't malloc Tx in glTexImage2D **\n");
    exit(1);
  }
  for(y=0; y<height; y++){
    Tx[y] = Tx_all + y*width;
  }
  for(y=0; y<height; y++){
    for(x=0; x<width; x++){
      for(c=0; c<4; c++){
	Tx[y][x][c] = ((unsigned char *)pixels + (y*width+x) * 4)[c];
      }
    }
  }
  TEX_HEIGHT = height;
  TEX_WIDTH  = width;
}

MYGLDEF_1 void MYGLDEF_2 glTexCoord2f( GLfloat s, GLfloat t )
{
  CurrentTexCoord[0] = s;
  CurrentTexCoord[1] = t;
}

MYGLDEF_1 void MYGLDEF_2 glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz )
{
  CurrentNormal[0] = nx;
  CurrentNormal[1] = ny;
  CurrentNormal[2] = nz;
}

MYGLDEF_1 void MYGLDEF_2 glTexParameteri( GLenum target, GLenum pname, GLint param )
{
  if(target != GL_TEXTURE_2D ||
     !((pname == GL_TEXTURE_MAG_FILTER && param == GL_NEAREST) ||
       (pname == GL_TEXTURE_MIN_FILTER && param == GL_NEAREST) ||
       (pname == GL_TEXTURE_WRAP_S && param == GL_CLAMP) || 
       (pname == GL_TEXTURE_WRAP_T && param == GL_CLAMP))){
    fprintf(stderr,"** warning : parameters are not supported in glTexParameteri **\n");
    //    exit(1);
  }
}

MYGLDEF_1 void MYGLDEF_2 glBlendFunc( GLenum sfactor, GLenum dfactor )
{
  if(sfactor != GL_SRC_ALPHA || dfactor != GL_ONE_MINUS_SRC_ALPHA){
    fprintf(stderr,"** error : parameters are not supported in glBlendFunc **\n");
    exit(1);
  }
}

MYGLDEF_1 void MYGLDEF_2 glAlphaFunc( GLenum func, GLclampf ref )
{
  if(func != GL_GREATER){
    fprintf(stderr, "** error : func is not supported in glAlphaFunc **\n");
    exit(1);
  }
  AlphaRef = ref;
}

MYGLDEF_1 void MYGLDEF_2 glEnable( GLenum cap )
{
  if(cap == GL_TEXTURE_2D){
    GlEnable_cap |= 1 << MYGL_TEXTURE;
  } else if(cap == GL_BLEND){
    GlEnable_cap |= 1 << MYGL_BLEND;
  } else if(cap == GL_DEPTH_TEST){
    GlEnable_cap |= 1 << MYGL_DEPTH_TEST;
  } else if(cap == GL_CULL_FACE){
    GlEnable_cap |= 1 << MYGL_CULL_FACE;
  } else if(cap == GL_ALPHA_TEST){
    GlEnable_cap |= 1 << MYGL_ALPHA_TEST;
  } else if(cap == GL_LIGHTING){
    GlEnable_cap |= 1 << MYGL_LIGHTING;
  } else if(cap == GL_LIGHT0){
    GlEnable_cap |= 1 << MYGL_LIGHT0;
  } else {
    fprintf(stderr,"** error : cap=%d is not supported in myglEnable **\n",cap);
    exit(1);      
  }
}

MYGLDEF_1 void MYGLDEF_2 glDisable( GLenum cap )
{
  if(cap == GL_TEXTURE_2D){
    GlEnable_cap &= ~(1 << MYGL_TEXTURE);
  } else if(cap == GL_BLEND){
    GlEnable_cap &= ~(1 << MYGL_BLEND);
  } else if(cap == GL_DEPTH_TEST){
    GlEnable_cap &= ~(1 << MYGL_DEPTH_TEST);
  } else if(cap == GL_CULL_FACE){
    GlEnable_cap &= ~(1 << MYGL_CULL_FACE);
  } else if(cap == GL_ALPHA_TEST){
    GlEnable_cap &= ~(1 << MYGL_ALPHA_TEST);
  } else if(cap == GL_LIGHTING){
    GlEnable_cap &= ~(1 << MYGL_LIGHTING);
  } else if(cap == GL_LIGHT0){
    GlEnable_cap &= ~(1 << MYGL_LIGHT0);
  } else {
    fprintf(stderr,"** error : cap=%d is not supported in myglEnable **\n",cap);
    exit(1);      
  }
}

MYGLDEF_1 void MYGLDEF_2 glGetFloatv( GLenum pname, GLfloat *params )
{
  int i;
  if(pname == GL_MODELVIEW_MATRIX){
    for(i=0; i<16; i++) params[i] = ((float *)Modelmatrix)[i];
  }
  else if(pname == GL_PROJECTION_MATRIX){
    for(i=0; i<16; i++) params[i] = ((float *)Projectmatrix)[i];
  } else {
    fprintf(stderr,"** error : pname is not supported in glGetFloatv **\n");
    exit(1);
  }
}

MYGLDEF_1 void MYGLDEF_2 glReadPixels( GLint x, GLint y,
				       GLsizei width, GLsizei height,
				       GLenum format, GLenum type,
				       GLvoid *pixels )
{
  int ix, iy, c;
  unsigned char *p = (unsigned char *)pixels;
  
  if(format != GL_RGB || type != GL_UNSIGNED_BYTE){
    fprintf(stderr,"** error : format=%d or type=%d is not supported in myglReadPixels **\n",format,type);
    exit(1);      
  }
  for(iy=0; iy<height; iy++){
    for(ix=0; ix<width; ix++){
      for(c=0; c<3; c++){
	p[(iy*width+ix)*3+c] = Fb[iy+y][ix+x][c];
      }
    }
  }
}

MYGLDEF_1 void MYGLDEF_2 glDrawPixels( GLsizei width, GLsizei height,
				       GLenum format, GLenum type,
				       const GLvoid *pixels )
{
  int ix, iy, c, nc;
  unsigned char *p = (unsigned char *)pixels;
  
  if(!(format == GL_RGB || format == GL_RGBA) || type != GL_UNSIGNED_BYTE){
    fprintf(stderr,"** error : format=%d or type=%d is not supported in glDrawPixels **\n",format,type);
    exit(1);      
  }
  if(format == GL_RGB)       nc = 3;
  else if(format == GL_RGBA) nc = 4;
  for(iy=0; iy<height; iy++){
    for(ix=0; ix<width; ix++){
      for(c=0; c<3; c++){
	Fb[iy][ix][c] = p[(iy*width + ix)*nc + c];
      }
    }
  }
}

MYGLDEF_1 void MYGLDEF_2 glMultMatrixf( const GLfloat *m )
{
  matrix_mul_4f(Currentmatrix, (float (*)[4])m, Currentmatrix);
}

MYGLDEF_1 void MYGLDEF_2 glMatrixMode( GLenum mode )
{
  if(mode == GL_MODELVIEW){
    Currentmatrix = Modelmatrix;
  }
  else if(mode == GL_PROJECTION){
    Currentmatrix = Projectmatrix;
  }
}

MYGLDEF_1 void MYGLDEF_2 glLoadIdentity( void )
{
  int i,j;
  for(i=0; i<4; i++){
    for(j=0; j<4; j++){
      if(i == j){
	Currentmatrix[i][j] = 1.0;
      } else {
	Currentmatrix[i][j] = 0.0;
      }
    }
  }
}

MYGLDEF_1 void MYGLDEF_2 glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of glTranslatef **\n");
    ini=1;
  }

    float T[4][4]= {{1.0,0.0,0.0,0.0},
                    {0.0,1.0,0.0,0.0},
                    {0.0,0.0,1.0,0.0},
                    {x  ,y  ,z  ,1.0}};
    matrix_mul_4f(Currentmatrix,T,Currentmatrix);
}

MYGLDEF_1 void MYGLDEF_2 glScalef( GLfloat x, GLfloat y, GLfloat z )
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of glScalef **\n");
    ini=1;
  }
    float S[4][4]= {{x  ,0.0,0.0,0.0},
                    {0.0,y  ,0.0,0.0},
                    {0.0,0.0,z  ,0.0},
                    {0.0,0.0,0.0,1.0}};
    matrix_mul_4f(Currentmatrix,S,Currentmatrix);
}

MYGLDEF_1 void MYGLDEF_2 glRotatef(GLfloat ang, GLfloat x, GLfloat y, GLfloat z){
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of glRotatef **\n");
    ini=1;
  }
    float r = ang/180*3.1415926535;
    float c=cos(r),s=sin(r);
    float l=x/sqrt(x*x+y*y+z*z),m=y/sqrt(x*x+y*y+z*z),n=z/sqrt(x*x+y*y+z*z);

    float R[4][4]= {{l*l+(1-l*l)*c, l*m*(1-c)+n*s, l*n*(1-c)-m*s, 0.0},
                    {l*m*(1-c)-n*s,  m*m+(1-m*m)*c, m*n*(1-c)+l*s,0.0},
                    {l*n*(1-c)+m*s, m*n*(1-c)-l*s, n*n+(1-n*n)*c, 0.0},
                    {0.0,           0.0,            0.0,          1.0}};

    matrix_mul_4f(Currentmatrix,R,Currentmatrix);
}

MYGLDEF_1 void MYGLDEF_2 glPushMatrix( void )
{
  // Copy Currentmatrix[][] to stack
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of glPushMatrix **\n");
    ini=1;
  }
    int i,j;
    for(i=0; i<4; i++){
        for(j=0; j<4; j++){
            Stackmatrix[Stackindex][i][j]=Currentmatrix[i][j];
        }
    }
    Stackindex++;
}

MYGLDEF_1 void MYGLDEF_2 glPopMatrix( void )
{
  // Copy from stack to Currentmatrix[][]
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of glPopMatrix **\n");
    ini=1;
  }
    int i,j;
    Stackindex--;
    for(i=0; i<4; i++){
        for(j=0; j<4; j++){
            Currentmatrix[i][j]=Stackmatrix[Stackindex][i][j];
        }
    }
}

MYGLDEF_1 void MYGLDEF_2 glOrtho( GLdouble left, GLdouble right, 
				  GLdouble bottom, GLdouble top,
				  GLdouble near_val, GLdouble far_val )
{
  /*  near_val = -near_val;
      far_val  = -far_val;  This function is already included by -2.0 in (3,3)
  */
  float m[4][4] = {{2.0/(right - left), 0.0, 0.0, 0.0},
		   {0.0, 2.0/(top - bottom), 0.0, 0.0},
		   {0.0, 0.0, -2.0/(far_val - near_val), 0.0},
		   {-(right + left)/(right - left), -(top + bottom)/(top - bottom), -(far_val + near_val)/(far_val - near_val), 1.0}};
  matrix_mul_4f(Currentmatrix,m,Currentmatrix);
}

MYGLDEF_1 void MYGLDEF_2 glFrustum( GLdouble left, GLdouble right,
				    GLdouble bottom, GLdouble top,
				    GLdouble near, GLdouble far )
{
  static int ini=0;
  if(ini==0){
    printf("** warning : you need to write your own code of glFrustum **\n");
    ini=1;
  }

    float P[4][4] ={{2*near/(right-left),0,0,0},
                    {0,2*near/(top-bottom),0,0},
                    {(right+left)/(right-left),(top+bottom)/(top-bottom),-(far+near)/(far-near),-1},
                    {0,0,-2*far*near/(far-near),0}};
    matrix_mul_4f(Currentmatrix,P,Currentmatrix);
}

void trans_vec(const float *src, float *dst, int n){
    int i;
    for(i=0; i<n; i++) dst[i]=src[i];
}

GLAPI void GLAPIENTRY glMaterialfv( GLenum face, GLenum pname, const GLfloat *params ){
    if(pname == GL_AMBIENT_AND_DIFFUSE){
        //printf("%f,%f,%f\n",params[0],params[1],params[2]);
        //printf("come here\n");
        trans_vec(params,Material_ambient,4);
        trans_vec(params,Material_diffuse,4);
    }else if(pname == GL_SPECULAR){
        trans_vec(params,Material_specular,4);
    }else{
        //unknown pname;
    }
}
GLAPI void GLAPIENTRY glMaterialf( GLenum face, GLenum pname, GLfloat param ){
    if(pname == GL_SHININESS){
        Material_shininess = param;
    }else{
        //unknown pname;
    }
}
GLAPI void GLAPIENTRY glLightfv( GLenum light, GLenum pname, const GLfloat *params ){
    if(pname == GL_AMBIENT){
        trans_vec(params,Light0_ambient,4);
    }else if(pname == GL_DIFFUSE){
        trans_vec(params,Light0_diffuse,4);
    }else if(pname == GL_SPECULAR){
        trans_vec(params,Light0_specular,4);
    }else if(pname == GL_POSITION){
        float buf[4];
        matrix_vec_4f(Currentmatrix,(float *)params,buf);
        trans_vec(buf,Light0_pos,4);
    }else{
        //unknown pname;
    }
}
GLAPI void GLAPIENTRY glLightModeli( GLenum pname, GLint param ){
    if(pname == GL_LIGHT_MODEL_LOCAL_VIEWER){
        LightModel_localviewer=param;
    }else{
        //unknown pname;
    }
}
GLAPI void GLAPIENTRY glLightModelfv( GLenum pname, const GLfloat *params ){
    if(pname == GL_LIGHT_MODEL_AMBIENT){
        trans_vec(params,Light0_la_ambient,4);
    }else{
        //unknown pname;
    }
}

#include "myglfw.c"
