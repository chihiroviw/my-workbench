#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>
//#include "glpng/include/GL/glpng.h"

GLFWwindow* Window;
GLFWwindow* Window_GL;

#define WIDTH  512
#define HEIGHT 512
#define ZOOM   16

unsigned char Fb[HEIGHT][WIDTH][3];
unsigned char Fb_GL[HEIGHT][WIDTH][3];

// Large texture
#define TEX_FNAME "uecphoto.data"
//#define TEX_FNAME "uecphoto_alpha.data"
#define TEX_HEIGHT 230
#define TEX_WIDTH  220
unsigned char Tx[TEX_HEIGHT][TEX_WIDTH][4];

void drawpointC(int x, int y, unsigned char *cl)
{
  int c;
  if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT){
    for(c=0;c<3;c++) Fb[y][x][c]=cl[c];
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

void drawtriangle(float x0, float y0, float x1, float y1, float x2, float y2)
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

void drawtriangleC(float x0, float y0, float x1, float y1, float x2, float y2,
		   unsigned char c0[], unsigned char c1[], unsigned char c2[])
{
  int minmax[2][2],x,y;
  float u,v;
  int c, cint;
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
	  cint = (c0[c] + u*(c1[c]-c0[c]) + v*(c2[c]-c0[c]) );
	  if(cint<0)         cl[c]=0;
	  else if(cint>=256) cl[c]=255;
          else               cl[c]=cint;
	}
	drawpointC(x,y,cl);
      }
    }
  }
}

void drawtriangleT(float x0, float y0, float x1, float y1, float x2, float y2,
		   float tx0, float ty0, float tx1, float ty1, float tx2, float ty2)
{
}

void drawtriangle_GL(float x0, float y0, float x1, float y1, float x2, float y2)
{
  glBegin(GL_TRIANGLES);

  glVertex2f(x0,y0);

  glVertex2f(x1,y1);

  glVertex2f(x2,y2);

  glEnd();
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

void drawtriangleT_GL(float x0, float y0, float x1, float y1, float x2, float y2,
		      float tx0, float ty0, float tx1, float ty1, float tx2, float ty2)
{
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLES);

  glTexCoord2f(tx0, ty0);
  glVertex2f(x0, y0);

  glTexCoord2f(tx1, ty1);
  glVertex2f(x1, y1);

  glTexCoord2f(tx2, ty2);
  glVertex2f(x2, y2);

  glEnd();
  glDisable(GL_TEXTURE_2D);
}

int main(int argc, char** argv)
{
  int x,y,c,i,j;
  int frame=0;
  
  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  Window_GL = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Triangle", NULL, NULL);
  Window = glfwCreateWindow(WIDTH, HEIGHT, "My Triangle", NULL, NULL);
  if (!Window_GL || !Window){
    glfwTerminate();
    return -1;
  }
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);

  // Initialize Texture
  glfwMakeContextCurrent(Window_GL);

  // Large texture
  FILE *fp;
  if((fp = fopen(TEX_FNAME,"rb"))!=NULL){
    fread(Tx, sizeof Tx, 1, fp);
    fclose(fp);
  } else {
    fprintf(stderr,"** error : can't read %s **\n",TEX_FNAME);
    exit(1);
  }
  // read png file
  /*
  pngRawInfo pnginfo;
  if(pngLoadRaw("uecphoto.png",&pnginfo) == 0 ||
     pnginfo.Width != TEX_WIDTH || pnginfo.Height != TEX_HEIGHT ||
     pnginfo.Components != 4 || pnginfo.Data==NULL){
    fprintf(stderr,"** error : reading png file failed **\n");
    exit(1);
  }
  for(y=0; y<TEX_HEIGHT; y++){
    for(x=0; x<TEX_WIDTH; x++){
      for(c=0; c<4; c++){
	Tx[y][x][c] = pnginfo.Data[(y*TEX_WIDTH+x)*4+c];
      }
    }
  }
  */

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_WIDTH, TEX_HEIGHT, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE, Tx);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(Window_GL) && !glfwWindowShouldClose(Window)){
    /* Start my routine */
    glfwMakeContextCurrent(Window);
    
    /* Clear frame buffer */
    for(y=0; y<HEIGHT; y++){
      for(x=0; x<WIDTH; x++){
	Fb[y][x][0] = 0;
	Fb[y][x][1] = 0;
	Fb[y][x][2] = 0;
      }
    }
    
    // Draw triangle
    drawtriangle(0.1+WIDTH/2,1.5,
		 7.1+WIDTH/2,0.1,
		 7.1+WIDTH/2,7.1);// front face

    // Draw texture
    for(y=0; y<TEX_HEIGHT; y++){
      for(x=0; x<TEX_WIDTH; x++){
	for(c=0; c<3; c++) Fb[y][x][c] = Tx[y][x][c];
      }
    }
    
    // Use different color for different vertex
    unsigned char cl[3][3]={{255,0,0}, {0,255,0}, {0,0,255}};
    drawtriangleC(WIDTH/2 +  20,  10,
		  WIDTH/2 + 200,  20, 
		  WIDTH/2 +  50, 200,
		  cl[0], cl[1], cl[2]);

    // Use texture
    drawtriangleT( 20,  10 + HEIGHT/2,
		  200,  20 + HEIGHT/2, 
		   50, 200 + HEIGHT/2,
		  0.0f, 0.0f,
		  1.0f, 0.0f,
		  1.0f, 1.0f);

    /* Zoom */
    for(y=0; y<HEIGHT/2; y++){
      for(x=0; x<WIDTH/2; x++){
	for(c=0; c<3; c++){
	  Fb[y + HEIGHT/2][x + WIDTH/2][c] = Fb[y/ZOOM][x/ZOOM+WIDTH/2][c];
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
    
    /* Clear frame buffer */
    glClear(GL_COLOR_BUFFER_BIT);

    /* Set range of draw area */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, WIDTH - 0.5 , -0.5, HEIGHT - 0.5, -1.0, 1.0);

    /* Setup culling face */
    glEnable(GL_CULL_FACE);
    glColor3ub(255, 255, 255);

    // Draw triangle
    drawtriangle_GL(0.1+WIDTH/2,1.5,
		    7.1+WIDTH/2,0.1,
		    7.1+WIDTH/2,7.1);// front face

    // Draw texture
    glDrawPixels(TEX_WIDTH,TEX_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,Tx);

    // Use different color for different vertex
    drawtriangleC_GL(WIDTH/2 +  20,  10,
		     WIDTH/2 + 200,  20, 
		     WIDTH/2 +  50, 200,
		     cl[0], cl[1], cl[2]);

    // Use texture
    glColor3ub(255, 255, 255);
    drawtriangleT_GL( 20,  10 + HEIGHT/2,
		     200,  20 + HEIGHT/2, 
		      50, 200 + HEIGHT/2,
		     0.0f, 0.0f,
		     1.0f, 0.0f,
		     1.0f, 1.0f);

    // Zoom
    // Copy from window to Fb_GL 
    glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,Fb_GL);
    for(y=0; y<HEIGHT/2; y++){
      for(x=0; x<WIDTH/2; x++){
	for(c=0; c<3; c++){
	  Fb_GL[y + HEIGHT/2][x + WIDTH/2][c] = Fb_GL[y/ZOOM][x/ZOOM+WIDTH/2][c];
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
