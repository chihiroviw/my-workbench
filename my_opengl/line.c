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

int Swap_xy = 0;
int Flip_y  = 0;

void drawpoint(int x, int y)
{
  int c;
  if(Swap_xy==0 && Flip_y==0){        // Normal case
    if(0<=x && x<WIDTH && 0<=y && y<HEIGHT){
      for(c=0; c<3; c++){
	Fb[y][x][c] = 255;
      }
    }
  } else if(Swap_xy==1 && Flip_y==0){ // swap x and y
    if(0<=x && x<HEIGHT && 0<=y && y<WIDTH){
      for(c=0; c<3; c++){
	Fb[x][y][c] = 255;
      }
    }
  } else if(Swap_xy==0 && Flip_y==1){ // y should be flipped
    if(0<=x && x<WIDTH && 0>=y && y>-HEIGHT){
      for(c=0; c<3; c++){
	Fb[-y][x][c] = 255;
      }
    }
  } else if(Swap_xy==1 && Flip_y==1){ // swap x,y and flip y
    if(0<=y && y<WIDTH && 0>=x && x>-HEIGHT){
      for(c=0; c<3; c++){
	Fb[-x][y][c] = 255;
      }
    }
  }
}

void drawline_sub(int x0, int y0, int x1, int y1)
{
  int x,y,e;
  x = x0;
  y = y0;
  e = -(x1 - x0);
  while(x<=x1){
    drawpoint(x,y);
    e += 2 * (y1 - y0);
    if(e >= 0){
      y++;
      e -= 2 * (x1 - x0);
    }
    x++;
  }
}

void drawline(int x0, int y0, int x1, int y1)
{
  if(x1<x0){
    drawline(x1,y1,x0,y0);
  } else {
    if(y1>=y0){
      if(y1 - y0 <= x1 - x0){  // example case 0 <= a <= 1
	drawline_sub(x0,y0,x1,y1);
      } else {                 // a > 1
	Swap_xy = 1;
	drawline_sub(y0,x0,y1,x1);
	Swap_xy = 0;
      }
    } else {
      Flip_y = 1;
      drawline(x0,-y0,x1,-y1);
      Flip_y = 0;
    }
  }
}

void drawline_GL(int x0, int y0, int x1, int y1)
{
  glBegin(GL_LINES);
  glVertex2i(x0,y0);
  glVertex2i(x1,y1);
  glEnd();
}

void drawcircle(int x0, int y0, int r)
{
  int x,y,d;
  x = 0;
  y = r;
  d = 3 - 2 * r;
  while(x<=y){
    drawpoint(x0+x, y0+y); drawpoint(x0-x, y0-y);
    drawpoint(x0+y, y0+x); drawpoint(x0-y, y0-x);
    drawpoint(x0+x, y0-y); drawpoint(x0-x, y0+y);
    drawpoint(x0-y, y0+x); drawpoint(x0+y, y0-x);
    if(d>=0){
      y--;
      d -= 4 * y;
    }
    x++;
    d += 4 * x + 2;
  }
}

void drawcircle_GL(int x0, int y0, int r)
{
}

int main(int argc, char** argv)
{
  int x,y,c,i,j;
  int frame=0;
  
  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  Window_GL = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Line", NULL, NULL);
  Window = glfwCreateWindow(WIDTH, HEIGHT, "My Line", NULL, NULL);
  if (!Window_GL || !Window){
    glfwTerminate();
    return -1;
  }
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);
  
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
    
    /* Draw line by myself */
    drawline(0,0,15,0);
    drawline(15,0,15,15);
    drawline(0,0,15,15);
    drawline(0,0,15,3);

    /*    
    // Draw rotating lines
    if(frame % (WIDTH*2)<WIDTH){
      drawline(frame % WIDTH,0,WIDTH-1-(frame % WIDTH),HEIGHT-1);
    } else {
      drawline(WIDTH-1,frame % WIDTH,0,HEIGHT-1-(frame % WIDTH));
    }
    */

    /*
    // Draw circle
    drawcircle(WIDTH/2,HEIGHT/2,frame % 256);
    */
    
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
    
    /* Clear frame buffer */
    glClear(GL_COLOR_BUFFER_BIT);

    /* 
    // Anti alias 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.5);
    */
    
    /* Set range of draw area */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, WIDTH - 0.5 , -0.5, HEIGHT - 0.5, -1.0, 1.0);
    
    /* Draw line */
    glColor3ub(255, 255, 255);
    drawline_GL(0,0,15,0);
    drawline_GL(15,0,15,15);
    drawline_GL(0,0,15,15);
    drawline_GL(0,0,15,3);

    /*
    // Draw rotating lines
    if(frame % (WIDTH*2)<WIDTH){
      drawline_GL(frame % WIDTH,0,WIDTH-1-(frame % WIDTH),HEIGHT-1);
    } else {
      drawline_GL(WIDTH-1,frame % WIDTH,0,HEIGHT-1-(frame % WIDTH));
    }
    */

    /*
    // Draw circle
    drawcircle_GL(WIDTH/2,HEIGHT/2,frame % 256);
    */

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
