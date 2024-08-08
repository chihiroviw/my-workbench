#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#define WIDTH  512
#define HEIGHT 512

GLFWwindow* Window;

#define ZOOM   16

unsigned char Fb_GL[HEIGHT][WIDTH][3];

//#define TEX_FNAME "uecphoto.data"
#define TEX_FNAME "uecphoto_alpha.data"
#define TEX_HEIGHT 230
#define TEX_WIDTH  220

/*
#define TEX_FNAME "halloween_alpha.data"
#define TEX_HEIGHT 232
#define TEX_WIDTH  218
*/

unsigned char Tx[TEX_HEIGHT][TEX_WIDTH][4];

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
  glColor3ub(255,255,255);
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

void drawtriangleCT_GL(float x0, float y0, float x1, float y1, float x2, float y2,
		       unsigned char c0[], unsigned char c1[], unsigned char c2[],
		       float tx0, float ty0, float tx1, float ty1, float tx2, float ty2)
{
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLES);

  glColor3ub(c0[0], c0[1], c0[2]);
  glTexCoord2f(tx0, ty0);
  glVertex2f(x0, y0);

  glColor3ub(c1[0], c1[1], c1[2]);
  glTexCoord2f(tx1, ty1);
  glVertex2f(x1, y1);

  glColor3ub(c2[0], c2[1], c2[2]);
  glTexCoord2f(tx2, ty2);
  glVertex2f(x2, y2);

  glEnd();
  glDisable(GL_TEXTURE_2D);
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
  Window = glfwCreateWindow(WIDTH, HEIGHT, "Sample1", NULL, NULL);
  if (!Window){
    glfwTerminate();
    return -1;
  }
  
  // Activate current Window 
  glfwMakeContextCurrent(Window);
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);
    
  // Large texture
  FILE *fp;
  if((fp = fopen(TEX_FNAME,"rb"))!=NULL){
    fread(Tx, sizeof Tx, 1, fp);
    fclose(fp);
  } else {
    fprintf(stderr,"** error : can't read %s **\n",TEX_FNAME);
    exit(1);
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_WIDTH, TEX_HEIGHT, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE, Tx);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //      glClearColor(0.0, 0.5, 0.5, 1.0);
  
  // Loop until the user closes the window 
  while (!glfwWindowShouldClose(Window)){
    // Clear frame buffer 
    glClearColor(0.0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable face culling
    glEnable(GL_CULL_FACE);

    // Set range of draw area 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //    glOrtho(-0.5, WIDTH - 0.5 , -0.5, HEIGHT - 0.5, -1.0, 1.0);
    glOrtho(-1.0, 1.0 , -1.0, 1.0, -1.0, 2.0);

    //    drawtriangle_GL(0.0,0.0,1.0,0.0,1.0,1.0);// front face
    drawtriangleC_GL(-1.0,-1.0,0.0,-1.0,0.0,0.0,cl[0],cl[1],cl[2]);
    glEnable(GL_BLEND);
    drawtriangleT_GL(-1.0,0.0,0.0,0.0,0.0,1.0,
		     0.0,0.0,1.0,0.0,1.0,1.0);
    glDisable(GL_BLEND);
    drawtriangleCT_GL(0.0,-1.0,1.0,-1.0,1.0,0.0,cl[0],cl[1],cl[2],
		      0.0,0.0,1.0,0.0,1.0,1.0);

    // Use Z-buffer
    glEnable(GL_DEPTH_TEST);
    p0[0] =   0/256.0; p0[1] = 4.2/256.0, p0[2] = -7/256.0;
    p1[0] = 180/256.0; p1[1] =  20/256.0, p1[2] = -7/256.0;
    p2[0] = 180/256.0; p2[1] = 150/256.0, p2[2] = -7/256.0;
    drawtriangleCZ_GL(p0, p1, p2, cl[0], cl[0], cl[1]);
    p0[0] = 0.8/256.0; p0[1] = 0.8/256.0, p0[2] = -9/256.0;
    p1[0] = 200/256.0; p1[1] =  50/256.0, p1[2] = -9/256.0;
    //    p2[0] =  50/256.0; p2[1] = 200/256.0, p2[2] = -9/256.0;
    p2[0] =  50/256.0; p2[1] = 200/256.0, p2[2] = -2/256.0;
    drawtriangleCZ_GL(p0, p1, p2, cl[1], cl[1], cl[2]);
    p0[0] = 4.2/256.0; p0[1] =   0/256.0, p0[2] = -8/256.0;
    p1[0] = 150/256.0; p1[1] = 180/256.0, p1[2] = -8/256.0;
    p2[0] =  20/256.0; p2[1] = 180/256.0, p2[2] = -8/256.0;
    drawtriangleCZ_GL(p0, p1, p2, cl[2], cl[2], cl[0]);
    glDisable(GL_DEPTH_TEST);

    
    glColor3ub(255, 255, 255);

    // Set range of draw area 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, WIDTH - 0.5 , -0.5, HEIGHT - 0.5, -1.0, 1.0);
#if 0    
    {
      float m[4][4];
      int i;
      glGetFloatv(GL_PROJECTION_MATRIX, (float *)m);
      for(i=0; i<4; i++){
	printf("%7.3f %7.3f %7.3f %7.3f\n",m[0][i],m[1][i],m[2][i],m[3][i]);
      }
      printf("\n");
    }
#endif

    // Draw triangle
    drawtriangle_GL(0.6,0.6,15.2,1.8,7.8,13.3);// front face
    //    drawtriangle_GL(0.6,0.6,7.8,13.3,15.2,1.8);// back face
    //    drawtriangleT_GL(0,0,7,0,7,7, 0.0f, 0.0f,1.0f, 0.0f,1.0f, 1.0f);
    //    drawtriangle_GL(0.1,1.5,7.1,0.1,7.1,7.1);// front face
    //    drawtriangleT_GL(6.1,7.5,13.1,6.1,13.1,13.1,0.0,0.0,1.0,0.0,1.0,1.0);// front face

    /*
    // Draw texture
    glDrawPixels(TEX_WIDTH,TEX_HEIGHT,GL_RGBA,GL_UNSIGNED_BYTE,Tx);
    */

    // Use different color for different vertex
    drawtriangleC_GL(0.0, HEIGHT/4,
		     TEX_WIDTH/2, HEIGHT/4,
		     TEX_WIDTH/2, HEIGHT/4 + TEX_HEIGHT/2,
		     cl[0], cl[1], cl[2]);
    
    /*
    // Use texture
    drawtriangleT_GL(0.0, HEIGHT/2,
		     TEX_WIDTH, HEIGHT/2,
		     TEX_WIDTH, HEIGHT/2 + TEX_HEIGHT,
		     0.0f, 0.0f,
		     1.0f, 0.0f,
		     1.0f, 1.0f);
    */
    /*
    drawtriangleCT_GL(0.0, HEIGHT/2,
		      TEX_WIDTH, HEIGHT/2,
		      TEX_WIDTH, HEIGHT/2 + TEX_HEIGHT,
		      cl[0], cl[1], cl[2],
		      0.0f, 0.0f,
		      1.0f, 0.0f,
		      1.0f, 1.0f);
    */

    // Wait until all GL functions are executed
    //    glFlush();
    
    /* Swap front and back buffers */
    glfwSwapBuffers(Window);
    
    /* Poll for and process events */
    glfwPollEvents();

    frame++;
  }
  
  glfwTerminate();
  
  return 0;
}
