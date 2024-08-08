/** Sabin Timalsena, Radiosity
 *
 * This program implements radiosity using the hemicube method to calculate the
 * form factors. To project facets to the sides of the hemicube, I have used
 * OpenGL's rasterizing functionality. All of this functionality is in the Scene
 * class, which holds the geometry of the scene and contains several functions.
 * The most important functions are loadFromObjFile(), which loads the geometry
 * from an external file; calculateFormFactors(), which calculates the form
 * factors for every face pair, and calculateRadiosities() which calculates the
 * radiosities based on form factors and face properties (one iteration). Also,
 * there are two rendering functions: render(), which renders the scene based on
 * the radiosities computed, and renderColorCoded(), which renders the scene
 * with a unique color for each face. Faces can be identified by the color drawn
 * into the framebuffer.
 *
 * The geometry consists entirely of triangles. They are read from a standard
 * .obj file, which lists vertices, faces, and also specifies which object a
 * group of faces belong to. The material properties are assigned properly to
 * each face based on the object name in the file. (Scene::loadFromObjFile())
 *
 * Application.h/cpp files contain all of the code for keyboard/mouse/event
 * handling. main.cpp sets up glut and register the handlers defined in the
 * Application class.
 *
 * Features:
 * -----------------------------------------------------------------------------
 * Press 'S' to start the simulation
 * Press 'X' to quit application
 * Pressing 'P' switches between orthographic/perspective renderings
 * Left mouse click + drag rotates the camera view
 */

//#include <GL/freeglut.h>
#include <GL/glut.h>
#include "Application.h"

// added by T.Narumi
#include <stdio.h>

static Application* application = 0;

static void keyboardFunc(unsigned char key, int x, int y) {
    application->keyboardFunc(key,x,y);
}

static void reshapeFunc(int w, int h) {
    application->reshapeFunc(w,h);
}

static void idleFunc() {
    application->idleFunc();
}

static void displayFunc(void) {
    application->displayFunc();
}

static void mouseMotionFunc (int x, int y) {
    application->mouseMotionFunc(x, y);
}

static void mouseFunc (int button, int state, int x, int y) {
    application->mouseFunc(button, state, x, y);
}

/* Program entry point */
int main(int argc, char *argv[]) {
    if(argc==1){ // added by T.Narumi
      printf("usage : %s obj_file_name (output_file_name) (command)(iterations) (scale)\n",argv[0]);
      printf("  output_file_name : extension is not needed\n");
      printf("  command & iterations : o30 -- iterate 30 times with original method\n");
      printf("                       : s30 -- iterate 30 times with gathering method\n");
      printf("                       : g500 - iterate 500 times with shooting method\n");
      printf("                       : t3 --- iterate 3 times with test parameters\n");
      printf("  scale : output obj is scaled with this number\n");
      printf("\n");
      printf("  ex. %s scene.obj scene_calc_g g30\n",argv[0]);
      printf("      %s scene.obj scene_calc_s s500\n",argv[0]);
      exit(1);
    }

    glutInit(&argc, argv);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);

    glutCreateWindow("Radiosity");

    // modified by T.Narumi: set output filename, command, etc.
    char ofname_obj[1024],ofname_mtl[1024];
    if(argc>=2){
      printf("load from %s\n",argv[1]);
      application = new Application(argv[1]);
      application->init();
    }
    if(argc>=3){
      float scale=1.0;
      sprintf(ofname_obj,"%s.obj",argv[2]);
      sprintf(ofname_mtl,"%s.mtl",argv[2]);
      printf("output files are %s and %s\n",ofname_obj,ofname_mtl);
      if(argc>=4){
	char com;
	int ite;
	sscanf(argv[3],"%c%d",&com,&ite);
	printf("command=%c, iterations=%d\n",com,ite);
	if(com!='o' && com!='s' && com!='g' && com!='t'){
	  fprintf(stderr,"** error : not supported command **\n");
	  exit(1);
	}
	application->set_command_iterations(com,ite);
      }
      if(argc>=5){
	sscanf(argv[4],"%f",&scale);
	printf("scale=%f\n",scale);
      }
      application->set_output_fname(ofname_obj,ofname_mtl,scale);
    }

    glutKeyboardFunc(keyboardFunc);
    glutReshapeFunc(reshapeFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(mouseMotionFunc);
    glutIdleFunc(idleFunc);
    glutDisplayFunc(displayFunc);

    displayFunc();
    glutMainLoop();

    delete application;
    return 0;
}
