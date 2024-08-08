using namespace std;
// simple code to compute Mandelbrot in C++
#include <complex>
void MandelbrotCPU(float x1, float y1, float x2, float y2, 
                   int width, int height, int maxIters, unsigned short * image)
{
  float dx = (x2-x1)/width, dy = (y2-y1)/height;
  for (int j = 0; j < height; ++j)
    for (int i = 0; i < width; ++i)
      {
	complex<float> c (x1+dx*i, y1+dy*j), z(0,0);
	int count = -1;
	while ((++count < maxIters) && (norm(z) < 4.0))
	  z = z*z+c;
	*image++ = count;
      }
}


#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

GLFWwindow* Window;

#define WIDTH  512
#define HEIGHT 512

unsigned char Fb[HEIGHT][WIDTH][3];
unsigned short Fb_short[HEIGHT*WIDTH];

int main(int argc, char** argv)
{
  float xc = (0.29768 + 0.29778) * 0.5;
  float yc = (0.48364 + 0.48354) * 0.5;
  float size = 3.0;
  int flag = 1;

  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  Window = glfwCreateWindow(WIDTH, HEIGHT, "Mandelbrot", NULL, NULL);
  if (!Window) {
    glfwTerminate();
    return -1;
  }
  
  /* Make the window's context current */
  glfwMakeContextCurrent(Window);
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);
  
  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(Window)) {
    float x1, x2, y1, y2;

    // update draw region 
    x1 = xc - size * 0.5;
    x2 = xc + size * 0.5;
    y1 = yc - size * 0.5;
    y2 = yc + size * 0.5;

    // Mandelbrot calculation
    MandelbrotCPU(x1,y1,x2,y2,WIDTH, HEIGHT,512,Fb_short);
    
    // Copy 16-bit color to 24-bit frame buffer
    int x,y;
    for(y=0; y<HEIGHT; y++){
      for(x=0; x<WIDTH; x++){
	/*
	Fb[y][x][0] = ((Fb_short[y*WIDTH + x] >> 8) & 0xf) << 4; // Red 
	Fb[y][x][1] = ((Fb_short[y*WIDTH + x] >> 4) & 0xf) << 4; // Green
	Fb[y][x][2] = ((Fb_short[y*WIDTH + x] >> 0) & 0xf) << 4; // Blue
	*/
	Fb[y][x][0] = ((Fb_short[y*WIDTH + x] >> 6) & 0x7) << 5; // Red 
	Fb[y][x][1] = ((Fb_short[y*WIDTH + x] >> 3) & 0x7) << 5; // Green
	Fb[y][x][2] = ((Fb_short[y*WIDTH + x] >> 0) & 0x7) << 5; // Blue
      }
    }
    
    /* Copy framebuffer to window area */
    glDrawPixels(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,Fb);
    
    /* Swap front and back buffers */
    glfwSwapBuffers(Window);
    
    /* Poll for and process events */
    glfwPollEvents();

    /* update size */
    if(flag>0) size *= 0.95;
    else       size /= 0.95;
    if(size<1e-4)   flag = -1;
    else if(size>3) flag = 1;
  }
  
  glfwTerminate();
  
  return 0;
}
