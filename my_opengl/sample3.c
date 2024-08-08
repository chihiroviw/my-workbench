#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glu.h>
#include "objread.h"

#define WIDTH  800
#define HEIGHT 800

GLFWwindow* Window;

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

static void set_texture(unsigned char *tx, int width, int height,
			int channel, int alpha_test)
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

int main(int argc, char** argv)
{
  int x,y,c,i,j;
  int frame=0;
  float cl[4][3]={{1,0,0}, {0,1,0}, {0,0,1}, {1,1,1}};
  float p0[3], p1[3], p2[3], p3[3], t0[2], t1[2], t2[2], t3[2];

  if(argc < 2){
    printf("usage : %s obj_file_name\n",argv[0]);
    return 1;
  }

  int ngroup, g, fg, f;
  t_obj obj;
  t_group *group;

  // read obj file and generate vertex and face information
  read_obj(argv[1],&obj);
  ngroup=obj.ngroup;
  group=obj.group;

  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  /* Create a windowed mode window and its OpenGL context */
  Window = glfwCreateWindow(WIDTH, HEIGHT, "Sample3", NULL, NULL);
  if (!Window){
    glfwTerminate();
    return -1;
  }
  
  // Activate current Window 
  glfwMakeContextCurrent(Window);
  
  // Set vsync(1;default) or not(0)
  glfwSwapInterval(1);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // Loop until the user closes the window 
  while (!glfwWindowShouldClose(Window)){
    // Clear frame buffer 
    glClearColor(0.0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set range of draw area 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)WIDTH / (double)HEIGHT, 0.1, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(4.0, 3.0, 3.0,     // 視点の位置
	      0.0, 2.0, 0.0,     // 見ている先
	      0.0, 1.0, 0.0);    // 視界の上方向

    glPushMatrix();
    glRotatef((frame % 3600)/10.0, 0.0, 1.0, 0.0);// rotate
    for(g=0; g<ngroup; g++){
      for(fg=0; fg<group[g].nfacegroup; fg++){
	t_vertex *v=obj.v;
	t_vnormal *vn=obj.vn;
	t_vtexture *vt=obj.vt;
	t_material *material=&group[g].fg[fg].material;
	if(material->txbuf!=NULL){ // with texture
	  set_texture(material->txbuf, material->tex_width, material->tex_height,
		      material->tex_channel, material->alpha_test);
              //printf("%d\n",material->alpha_test);
	  for(f=0; f<group[g].fg[fg].nface; f++){
	    t_face *tf=&group[g].fg[fg].f[f];
	    drawtriangleCTZ_GL(v[ tf->p[0] ].p, v[ tf->p[1] ].p, v[ tf->p[2] ].p,
			       cl[3], cl[3], cl[3],
			       vt[ tf->t[0] ].t, vt[ tf->t[1] ].t, vt[ tf->t[2] ].t);
	  }
	} else {                   // without texture
	  for(f=0; f<group[g].fg[fg].nface; f++){
	    t_face *tf=&group[g].fg[fg].f[f];
	    drawtriangleCZ_GL(v[ tf->p[0] ].p, v[ tf->p[1] ].p, v[ tf->p[2] ].p,
			      material->Kd, material->Kd, material->Kd);
	  }
	}
      }
    }
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
