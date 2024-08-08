#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>

#define WIDTH  500
#define HEIGHT 500

GLFWwindow* Window;

void print_matrix(void)
{
  float matrix[4][4];
  int i,j;
  
  glGetFloatv(GL_MODELVIEW_MATRIX,(GLfloat *)matrix[0]);
  printf(" - %7.1f %7.1f %7.1f %7.1f -\n", matrix[0][0],matrix[1][0],matrix[2][0],matrix[3][0]);
  printf("|  %7.1f %7.1f %7.1f %7.1f  |\n",matrix[0][1],matrix[1][1],matrix[2][1],matrix[3][1]);
  printf("|  %7.1f %7.1f %7.1f %7.1f  |\n",matrix[0][2],matrix[1][2],matrix[2][2],matrix[3][2]);
  printf(" - %7.1f %7.1f %7.1f %7.1f -\n", matrix[0][3],matrix[1][3],matrix[2][3],matrix[3][3]);
}

void drawtriangle_GL(float x0, float y0, float x1, float y1, float x2, float y2)
{
  glBegin(GL_TRIANGLES);
  glVertex2f(x0,y0);
  glVertex2f(x1,y1);
  glVertex2f(x2,y2);
  glEnd();
}

int main(int argc, char** argv)
{
  int x,y,c,i,j;
  int frame=0;
  
  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  Window = glfwCreateWindow(WIDTH, HEIGHT, "Sample2", NULL, NULL);
  if (!Window){
    glfwTerminate();
    return -1;
  }
  
  // Activate current Window 
  glfwMakeContextCurrent(Window);
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);
    
  // Loop until the user closes the window 
  while (!glfwWindowShouldClose(Window)){
    // Clear frame buffer 
    glClearColor(0.0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3ub(255,255,255);

    // Set range of draw area 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1.0, 1.0);

    // Set model projection matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw grid lines
    for(i=0; i<500; i+=100){
      drawtriangle_GL(i, 0, i+1, 0, i+1, 500);
      drawtriangle_GL(i, 0, i+1, 500, i, 500);
      drawtriangle_GL(0, i, 500, i+1,   0, i+1);
      drawtriangle_GL(0, i, 500, i,   500, i+1);
    }
    
    // Insert scale, transform, or rotate here

    // glTranslatef(100, 0, 0);
     //glRotatef(45, 0, 0, 1);
     //glScalef(1, 2, 1);

    drawtriangle_GL(0, 0, 200, 0, 200, 100);// front face

    
    glPushMatrix();
    glTranslatef(0, 200, 0);
    if(frame % 1000==0) print_matrix();
    drawtriangle_GL(0, 0, 200, 0, 200, 100);// front face
    glPopMatrix();
    glPushMatrix();
    glTranslatef(200, 200, 0);
    if(frame % 1000==0) print_matrix();
    drawtriangle_GL(0, 0, 200, 0, 200, 100);// front face
    glPopMatrix();
    
    
    /* Swap front and back buffers */
    glfwSwapBuffers(Window);
    
    /* Poll for and process events */
    glfwPollEvents();

    frame++;
  }
  
  glfwTerminate();
  
  return 0;
}
