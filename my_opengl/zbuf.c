#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

GLFWwindow* Window;
GLFWwindow* Window_GL;

#define WIDTH  512
#define HEIGHT 512
#define ZOOM   16

unsigned char Fb[HEIGHT][WIDTH][3];
unsigned char Fb_GL[HEIGHT][WIDTH][3];
float Zb[HEIGHT][WIDTH];

void drawpointC(int x, int y, unsigned char *cl)
{
  int c;
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT){
    for(c=0;c<3;c++) Fb[y][x][c]=cl[c];
  }
}

void drawpointCZ(int x, int y, float z, unsigned char *cl)
{
  int c;
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT){
    if(z<Zb[y][x]){
      for(c=0;c<3;c++) Fb[y][x][c]=cl[c];
      Zb[y][x] = z;
    }
  }
}

void drawpoint(int x, int y)
{
  unsigned char c[]={255,255,255};
  drawpointC(x,y,c);
}

int drawtriangle_sub(int x, int y, float x0, float y0, float x1, float y1)
{
  /* 0 -- right hand side of line (x0,y0)-(x1,y1)
     1 -- left hand side of line (x0,y0)-(x1,y1)
  */
  if((x1-x0)*(y-y0)-(y1-y0)*(x-x0)>=0.0f) return 1;
  else                                    return 0;
}

void drawtriangle_calcminmax(float x0,  float y0,
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
}

void drawtriangleC(float x0, float y0, float x1, float y1, float x2, float y2,
		   unsigned char c0[], unsigned char c1[], unsigned char c2[])
{
  int minmax[2][2],x,y;
  float u,v;
  int c;
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
	  cl[c] = (c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]) );
	  if(cl[c]<0)         cl[c]=0;
	  else if(cl[c]>=256) cl[c]=255;
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
  int c;
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
	for(c=0; c<3; c++){
	  cl[c] = (c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]) );
	  if(cl[c]<0)         cl[c]=0;
	  else if(cl[c]>=256) cl[c]=255;
	}
	z = p0[2] + u*(p1[2]-p0[2]) + v*(p2[2]-p0[2]);
	drawpointCZ(x,y,z,cl);
      }
    }
  }
}

void drawtriangleC_GL(float x0, float y0, float x1, float y1, float x2, float y2,
		      unsigned char c0[], unsigned char c1[], unsigned char c2[])
{
  glBegin(GL_TRIANGLES);

  glColor3ub(c0[0], c0[1], c0[2]);
  glVertex2f(x0, y0);

  glColor3ub(c1[0], c1[1], c1[2]);
  glVertex2f(x1, y1);

  glColor3ub(c2[0], c2[1], c2[2]);
  glVertex2f(x2, y2);

  glEnd();
}

void drawtriangleCZ_GL(float p0[], float p1[], float p2[],
		       unsigned char c0[], unsigned char c1[], unsigned char c2[])
{
  glBegin(GL_TRIANGLES);

  glColor3ub(c0[0], c0[1], c0[2]);
  glVertex3f(p0[0], p0[1], p0[2]);

  glColor3ub(c1[0], c1[1], c1[2]);
  glVertex3f(p1[0], p1[1], p1[2]);

  glColor3ub(c2[0], c2[1], c2[2]);
  glVertex3f(p2[0], p2[1], p2[2]);

  glEnd();
}

int main(int argc, char** argv)
{
  int x,y,c,i,j;
  int frame=0;
  unsigned char cl[3][3]={{255,0,0}, {0,255,0}, {0,0,255}};
  float p0[3], p1[3], p2[3];
  
  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  Window_GL = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Z-buffer", NULL, NULL);
  Window = glfwCreateWindow(WIDTH, HEIGHT, "My Z-buffer", NULL, NULL);
  if (!Window_GL || !Window){
    glfwTerminate();
    return -1;
  }
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);

  // Initialize Texture
  glfwMakeContextCurrent(Window_GL);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(Window_GL) && !glfwWindowShouldClose(Window)){
    /* Start my routine */
    glfwMakeContextCurrent(Window);
    
    /* Clear frame buffer and Z-buffer*/
    for(y=0; y<HEIGHT; y++){
      for(x=0; x<WIDTH; x++){
	Fb[y][x][0] = 0;
	Fb[y][x][1] = 0;
	Fb[y][x][2] = 0;
	Zb[y][x]    = 10.0f;
      }
    }

    // Use Z-buffer
    p0[0] =   0; p0[1] = 4.2, p0[2] = 7;
    p1[0] = 180; p1[1] =  20, p1[2] = 7;
    p2[0] = 180; p2[1] = 150, p2[2] = 7;
    drawtriangleCZ(p0, p1, p2, cl[0], cl[0], cl[0]);
    p0[0] = 0.8; p0[1] = 0.8, p0[2] = 9;
    p1[0] = 200; p1[1] =  50, p1[2] = 9;
    p2[0] =  50; p2[1] = 200, p2[2] = 2;
    drawtriangleCZ(p0, p1, p2, cl[1], cl[1], cl[1]);
    p0[0] = 4.2; p0[1] =   0, p0[2] = 8;
    p1[0] = 150; p1[1] = 180, p1[2] = 8;
    p2[0] =  20; p2[1] = 180, p2[2] = 8;
    drawtriangleCZ(p0, p1, p2, cl[2], cl[2], cl[2]);

    /* Zoom */
    for(y=0; y<HEIGHT/2; y++){
      for(x=0; x<WIDTH/2; x++){
	for(c=0; c<3; c++){
	  Fb[y + HEIGHT/2][x + WIDTH/2][c] = Fb[y/ZOOM][x/ZOOM][c];
	}
      }
    }
    /* Copy from Fb to window */
    glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,Fb);
    
    /* Swap front and back buffers */
    glfwSwapBuffers(Window);

    /* End my routine */


    /* Start Open GL routine */
    glfwMakeContextCurrent(Window_GL);
    
    /* Clear frame buffer and Z-buffer*/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Set range of draw area */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, WIDTH - 0.5 , -0.5, HEIGHT - 0.5, 0.0, -10.0);

    /* Setup culling face */
    glEnable(GL_CULL_FACE);

    // Use Z-buffer
    glEnable(GL_DEPTH_TEST);
    p0[0] =   0; p0[1] = 4.2, p0[2] = 7;
    p1[0] = 180; p1[1] =  20, p1[2] = 7;
    p2[0] = 180; p2[1] = 150, p2[2] = 7;
    drawtriangleCZ_GL(p0, p1, p2, cl[0], cl[0], cl[0]);
    p0[0] = 0.8; p0[1] = 0.8, p0[2] = 9;
    p1[0] = 200; p1[1] =  50, p1[2] = 9;
    p2[0] =  50; p2[1] = 200, p2[2] = 2;
    drawtriangleCZ_GL(p0, p1, p2, cl[1], cl[1], cl[1]);
    p0[0] = 4.2; p0[1] =   0, p0[2] = 8;
    p1[0] = 150; p1[1] = 180, p1[2] = 8;
    p2[0] =  20; p2[1] = 180, p2[2] = 8;
    drawtriangleCZ_GL(p0, p1, p2, cl[2], cl[2], cl[2]);
    
    // Zoom
    // Copy from window to Fb_GL 
    glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,Fb_GL);
    for(y=0; y<HEIGHT/2; y++){
      for(x=0; x<WIDTH/2; x++){
	for(c=0; c<3; c++){
	  Fb_GL[y + HEIGHT/2][x + WIDTH/2][c] = Fb_GL[y/ZOOM][x/ZOOM][c];
	}
      }
    }
    // Copy from Fb_GL to window 
    glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,Fb_GL);
    
    /* Swap front and back buffers */
    glfwSwapBuffers(Window_GL);
    
    /* End Open GL routine */
    
    
    /* Poll for and process events */
    glfwPollEvents();

    frame++;
  }
  
  glfwTerminate();
  
  return 0;
}
