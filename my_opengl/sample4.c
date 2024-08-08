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

void drawtriangleNZ_GL(float p0[3], float p1[3], float p2[3],
		       float n0[3], float n1[3], float n2[3])
{
  glBegin(GL_TRIANGLES);

  glNormal3f(n0[0], n0[1], n0[2]);
  glVertex3f(p0[0], p0[1], p0[2]);

  glNormal3f(n1[0], n1[1], n1[2]);
  glVertex3f(p1[0], p1[1], p1[2]);

  glNormal3f(n2[0], n2[1], n2[2]);
  glVertex3f(p2[0], p2[1], p2[2]);

  glEnd();
}

void drawtriangleNTZ_GL(float p0[3], float p1[3], float p2[3],
			float n0[3], float n1[3], float n2[3],
			float t0[2], float t1[2], float t2[2])
{
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_TRIANGLES);

  glNormal3f(n0[0], n0[1], n0[2]);
  glTexCoord2f(t0[0], t0[1]);
  glVertex3f(p0[0], p0[1], p0[2]);

  glNormal3f(n1[0], n1[1], n1[2]);
  glTexCoord2f(t1[0], t1[1]);
  glVertex3f(p1[0], p1[1], p1[2]);

  glNormal3f(n2[0], n2[1], n2[2]);
  glTexCoord2f(t2[0], t2[1]);
  glVertex3f(p2[0], p2[1], p2[2]);

  glEnd();
  glDisable(GL_TEXTURE_2D);
}

static void set_alphatest(int alpha_test)
{
  if(alpha_test==1){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.3);
  } else if(alpha_test==0){
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }
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
      t_vnormal *vn=obj.vn;
      t_vtexture *vt=obj.vt;
      t_material *material=&group[g].fg[fg].material;
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material->Kd);
      glMaterialfv(GL_FRONT, GL_SPECULAR, material->Ks);
      glMaterialf(GL_FRONT, GL_SHININESS, material->Ns);
      if(material->txbuf!=NULL){ // with texture
	set_texture(material->txbuf,material->tex_width,material->tex_height,material->alpha_test);
	set_alphatest(material->alpha_test);
	for(f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  drawtriangleNTZ_GL(v[ tf->p[0] ].p, v[ tf->p[1] ].p, v[ tf->p[2] ].p,
			     vn[ tf->n[0] ].n, vn[ tf->n[1] ].n, vn[ tf->n[2] ].n,
			     vt[ tf->t[0] ].t, vt[ tf->t[1] ].t, vt[ tf->t[2] ].t);
	}
      } else {                   // without texture
	for(f=0; f<group[g].fg[fg].nface; f++){
	  t_face *tf=&group[g].fg[fg].f[f];
	  drawtriangleNZ_GL(v[ tf->p[0] ].p, v[ tf->p[1] ].p, v[ tf->p[2] ].p,
			    vn[ tf->n[0] ].n, vn[ tf->n[1] ].n, vn[ tf->n[2] ].n);
	}
      }
    }
  }
}

int main(int argc, char** argv)
{
  int x,y,c,i,j,width,height;
  int frame=0;
  float cl[4][3]={{1,0,0}, {0,1,0}, {0,0,1}, {1,1,1}};
  float p0[3], p1[3], p2[3], p3[3], t0[2], t1[2], t2[2], t3[2];
  t_obj obj;

  if(argc < 2){
    printf("usage : %s obj_file_name\n",argv[0]);
    return 1;
  }

  // read obj file and generate vertex and face information
  read_obj(argv[1],&obj);
  generate_normal(&obj); // generate normal vector of faces

  /* Initialize the library */
  if (!glfwInit())
    return -1;
  
  // Create a windowed mode window
  width = WIDTH;
  height = HEIGHT;
  Window = glfwCreateWindow(width, height, "Sample4", NULL, NULL);

  // Check if a window is opened or not
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

  // Enable lighting
  float lightpos[4]={4,4,1,1}, lm_ambient[4]={0,0,0,1}, ambient[4]={0.2,0.2,0.2,1};
  float diffuse[4]={1,1,1,1}, specular[4]={3,3,3,1};
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
 
  // Loop until the user closes the window 
  while (!glfwWindowShouldClose(Window)){
    // Clear frame buffer 
    glClearColor(0.0, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set range of draw area 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);

    // Set camera position and direction
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(4.0, 3.0, 3.0, 0.0, 2.0, 0.0, 0.0, 1.0, 0.0);

    // Set light position
    glPushMatrix();
    //glRotatef(frame % 360, 0.0, 1.0, 0.0);// rotate
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos); // set light position
    glPopMatrix();

    // Draw object
    glPushMatrix();
    glRotatef((frame % 3600)/10.0, 0.0, 1.0, 0.0);// rotate
    draw_object(obj);              // draw object
    //    glCallList(1);
    glPopMatrix();
 
    /* Swap front and back buffers */
    glfwSwapBuffers(Window);
    
    /* Poll for and process events */
    glfwPollEvents();

    frame +=20;
  }
  
  glfwTerminate();
  
  return 0;
}
