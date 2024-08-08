#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glu.h>
#include "objread.h"

//#define FULL_SCREEN // Define this if you want full screen mode
#define WIDTH  1200
#define HEIGHT 800

GLFWwindow* Window;

void make_movie_initialize(const char *fname, int width, int height);
void make_movie_oneframe(unsigned char *rawdata);

void drawtriangleCZ_GL(float p0[3], float p1[3], float p2[3],
		       float c0[3], float c1[3], float c2[3])
{
  glBegin(GL_TRIANGLES);

  glColor3f(c0[0], c0[1], c0[2]);
  glVertex3f(p0[0], p0[1], p0[2]);

  glColor3f(c1[0], c1[1], c1[2]);
  glVertex3f(p1[0], p1[1], p1[2]);

  glColor3f(c2[0], c2[1], c2[2]);
  glVertex3f(p2[0], p2[1], p2[2]);

  glEnd();
}

void drawtriangleCTZ_GL(float p0[3], float p1[3], float p2[3],
			float c0[3], float c1[3], float c2[3],
			float t0[2], float t1[2], float t2[2])
{
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLES);

  glColor3f(c0[0], c0[1], c0[2]);
  glTexCoord2f(t0[0], t0[1]);
  glVertex3f(p0[0], p0[1], p0[2]);

  glColor3f(c1[0], c1[1], c1[2]);
  glTexCoord2f(t1[0], t1[1]);
  glVertex3f(p1[0], p1[1], p1[2]);

  glColor3f(c2[0], c2[1], c2[2]);
  glTexCoord2f(t2[0], t2[1]);
  glVertex3f(p2[0], p2[1], p2[2]);

  glEnd();
  glDisable(GL_TEXTURE_2D);
}

static void set_texture(unsigned char *tx, int width, int height, int alpha_test)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
	       GL_RGBA, GL_UNSIGNED_BYTE, tx);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  if(alpha_test==1){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.3);
  } else if(alpha_test==0){
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }
}

void draw_object(t_obj obj)
{
  int ngroup, g, fg, f;
  t_group *group;
  float cl[4][3]={{1,0,0}, {0,1,0}, {0,0,1}, {1,1,1}};

  ngroup=obj.ngroup;
  group=obj.group;
  for(g=0; g<ngroup; g++){
    for(fg=0; fg<group[g].nfacegroup; fg++){
      t_vertex *v=obj.v;
      t_vtexture *vt=obj.vt;
      t_material *material=&group[g].fg[fg].material;
      if(material->txbuf!=NULL){ // with texture
	set_texture(material->txbuf,material->tex_width,material->tex_height,material->alpha_test);
	for(f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  drawtriangleCTZ_GL(v[ tf->p[0] ].p, v[ tf->p[1] ].p, v[ tf->p[2] ].p,
			     cl[3], cl[3], cl[3],
			     vt[ tf->t[0] ].t, vt[ tf->t[1] ].t, vt[ tf->t[2] ].t);
	}
	glDisable(GL_ALPHA_TEST);
      } else {                   // without texture
	for(f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  drawtriangleCZ_GL(v[ tf->p[0] ].p, v[ tf->p[1] ].p, v[ tf->p[2] ].p,
			    material->Kd, material->Kd, material->Kd);
	}
      }
    }
  }
}

void display(int width, int height, t_obj obj,
	     int frame, int left_right)
{
    // Set range of draw area 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
  
    // Set camera position and direction
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  
    // Draw object for right or left eye
    if(left_right==-1){        // for left eye  
        gluLookAt(  4.0-(3.0/5.0)*0.4, 3.0, 3.0+(4.0/5.0)*0.4, 
                    0.0, 2.0, 0.0, 
                    0.0, 1.0, 0.0);   
    } else if(left_right==1){  // for right eye
        gluLookAt(  4.0+(3.0/5.0)*0.4, 3.0, 3.0-(4.0/5.0)*0.4, 
                    0.0, 2.0, 0.0, 
                    0.0, 1.0, 0.0);
    }
    glRotatef(frame % 360, 0.0, 1.0, 0.0);
    draw_object(obj);          // draw object
}

int main(int argc, char** argv)
{
  int x,y,c,i,j,width,height;
  int frame=0;
  float cl[4][3]={{1,0,0}, {0,1,0}, {0,0,1}, {1,1,1}};
  float p0[3], p1[3], p2[3], p3[3], t0[2], t1[2], t2[2], t3[2];
  t_obj obj;

  if(argc < 2){
    printf("usage : %s obj_file_name (movie_file_name)\n",argv[0]);
    return 1;
  }

  // read obj file and generate vertex and face information
  read_obj(argv[1],&obj);

  /* Initialize the library */
  if (!glfwInit())
    return -1;

  char title[1024];
  sprintf(title,"Stereo : %s",argv[1]);
#ifdef FULL_SCREEN  
  // Create a full screen mode window
  int mcount;
  GLFWmonitor **monitors = glfwGetMonitors(&mcount);
  GLFWmonitor *primary;
  const GLFWvidmode *mode;
  if(mcount==1) primary = glfwGetPrimaryMonitor();
  else          primary = monitors[1];// choose secondary monitor
  mode = glfwGetVideoMode(primary);
  width = mode->width;
  height = mode->height;
  Window = glfwCreateWindow(width, height, title, primary, NULL);
#else
  // Create a windowed mode window
  width = WIDTH;
  height = HEIGHT;
  Window = glfwCreateWindow(width, height, title, NULL, NULL);
#endif  

  // Check if a window is opened or not
  if (!Window){
    glfwTerminate();
    return -1;
  }
  
  // setup output movie file
  if(argc >= 3){
    printf("output movie file is %s\n",argv[2]);
    make_movie_initialize(argv[2],width,height);
  }

  // Activate current Window 
  glfwMakeContextCurrent(Window);
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);

  // Enable GL settings
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // Prepare frame buffers for right and left images
  unsigned char *fb_r=(unsigned char *)malloc(sizeof(unsigned char)*width*height*3);
  unsigned char *fb_l=(unsigned char *)malloc(sizeof(unsigned char)*width*height*3);
  unsigned char *fb_rl=(unsigned char *)malloc(sizeof(unsigned char)*width*height*3);
  
  // Loop until the user closes the window 
  while (!glfwWindowShouldClose(Window)){
    // Set clear color 
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Display left image and get pixel data 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    display(width,height,obj,frame,-1 /* left */);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,fb_l);

    // Display right image and get pixel data 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    display(width,height,obj,frame,1 /* right */);
    glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,fb_r);

    // Merge left and right images into fb_l
    for(y=0; y<height; y++){
        for(x=0; x<width/2; x++){
            int rgb;
            for(rgb=0; rgb<3; rgb++){
                fb_rl[(y*width+x)*3+rgb] = fb_l[(y*width+x+width/4)*3+rgb];
            }
        }
    }
    for(y=0; y<height; y++){
        for(x=0; x<width/2; x++){
            int rgb;
            for(rgb=0; rgb<3; rgb++){
                fb_rl[(y*width+x+width/2)*3+rgb] = fb_r[(y*width+x+width/4)*3+rgb];
            }
        }
    }
    // draw merged image
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,fb_rl);
    glEnable(GL_DEPTH_TEST);
  
    // output movie file
    if(argc >= 3){
      if(frame<360) make_movie_oneframe(fb_rl);
      else if(frame==360){
	printf("Maximum number of movie frames is limited to 360\n");
      }
    }
  
    /* Swap front and back buffers */
    glfwSwapBuffers(Window);
    
    /* Poll for and process events */
    glfwPollEvents();

    frame++;
  }
  
  glfwTerminate();
  
  return 0;
}
