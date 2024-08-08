#ifdef _WIN32
#include <windows.h>
#define dlsym(D,F) (void*)GetProcAddress((HMODULE)D,F)
#else
#include <dlfcn.h>    // for calling dynamic link library
#endif

MYGLDEF_1 void MYGLDEF_2 gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
  double ymax, xmax;
  ymax = zNear * tan(fovy*M_PI/180.0/2.0);
  xmax = aspect * ymax;
  glFrustum(-xmax, xmax, -ymax, ymax, zNear, zFar);
}

/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 */
MYGLDEF_1 void MYGLDEF_2 gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx,
				   GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy,
				   GLdouble upz)
{
  float forward[3], side[3], up[3];
  GLfloat m[4][4];
  
  forward[0] = centerx - eyex;
  forward[1] = centery - eyey;
  forward[2] = centerz - eyez;
  
  up[0] = upx;
  up[1] = upy;
  up[2] = upz;
  
  normalize(forward);
  
  /* Side = forward x up */
  cross(forward, up, side);
  normalize(side);
  
  /* Recompute up as: up = side x forward */
  cross(side, forward, up);

  //  __gluMakeIdentityf(&m[0][0]);
  {
    int i,j;
    for(i=0;i<4;i++) for(j=0;j<4;j++) m[i][j]=0.0f;
    for(i=0;i<4;i++) m[i][i]=1.0f;
  }
  m[0][0] = side[0];
  m[1][0] = side[1];
  m[2][0] = side[2];
  
  m[0][1] = up[0];
  m[1][1] = up[1];
  m[2][1] = up[2];
  
  m[0][2] = -forward[0];
  m[1][2] = -forward[1];
  m[2][2] = -forward[2];
  
  glMultMatrixf(&m[0][0]);
  glTranslatef(-eyex, -eyey, -eyez);
}

static void allocate_buffers(int width, int height)
{
  int y;
  if(Fb != NULL){
    free(Fb);
    free(Fb_all);
    free(Zb);
    free(Zb_all);
  }
  if((Fb_all=(unsigned char (*)[3])malloc(sizeof(unsigned char)*3*width*height))==NULL){
    fprintf(stderr,"** can't malloc Fb_all in glfwCreateWindow **\n");
    exit(1);
  }
  if((Fb=(unsigned char (**)[3])malloc(sizeof(unsigned char (*)[3])*height))==NULL){
    fprintf(stderr,"** can't malloc Fb in glfwCreateWindow **\n");
    exit(1);
  }
  if((Zb_all=(float *)malloc(sizeof(float)*width*height))==NULL){
    fprintf(stderr,"** can't malloc Zb_all in glfwCreateWindow **\n");
    exit(1);
  }
  if((Zb=(float **)malloc(sizeof(float *)*height))==NULL){
    fprintf(stderr,"** can't malloc Zb in glfwCreateWindow **\n");
    exit(1);
  }
  for(y=0; y<height; y++){
    Fb[y] = Fb_all + y*width;
    Zb[y] = Zb_all + y*width;
  }
}

static GLFWwindow *Myglfwwindow=NULL;

GLFWAPI GLFWwindow* glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
  printf("in glfwCreateWindow, width=%d height=%d\n",width,height);
  GLFWwindow *(*original_glfwCreateWindow)(int, int, const char*, GLFWmonitor*, GLFWwindow*);
  WIDTH = width;
  HEIGHT = height;
  allocate_buffers(width, height);
  static char mygltitle[1024]="Mygl ";
  strncpy(mygltitle+5,title,1000);mygltitle[1024-1]='\0';
  original_glfwCreateWindow = dlsym(RTLD_NEXT, "glfwCreateWindow");
  Myglfwwindow = (*original_glfwCreateWindow)(width, height, mygltitle, monitor, share);
  return Myglfwwindow;
}

GLFWAPI void glfwSwapBuffers(GLFWwindow* window)
{
  if(Myglfwwindow == window){
    void (*original_glDrawPixels)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *)
      = dlsym(RTLD_NEXT, "glDrawPixels");
    (*original_glDrawPixels)(WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,Fb_all);
  }
  void (*original_glfwSwapBuffers)(GLFWwindow*) = dlsym(RTLD_NEXT, "glfwSwapBuffers");
  (*original_glfwSwapBuffers)(window);
}
