#include "Application.h"
#include <GL/freeglut.h>
#include <fstream>
#include <iostream>

using namespace std;

Application::Application() {
    isSimulating = false;
    isGather = false; // added by T.Narumi
    isProgressive = false; // added by T.Narumi
    numIterations = 0;

    camera.position = vec3(15, 15, 15);
    camera.pitch = -20;
    camera.yaw = 45;
    perspective = false;

    scene.loadFromOBJFile("scene.obj");
    scene.buildVertexToFaceMap();
    scene.autoCalculateNormals();
}

Application::Application(const char *fname) {
    isSimulating = false;
    isGather = false; // added by T.Narumi
    isProgressive = false; // added by T.Narumi
    numIterations = 0;

    camera.position = vec3(15, 15, 15);
    camera.pitch = -20;
    camera.yaw = 45;
    perspective = false;

    scene.loadFromOBJFile(fname);
    scene.buildVertexToFaceMap();
    scene.autoCalculateNormals();
}

// added by T.Narumi
void Application::set_output_fname(const char *ofname_obj, const char *ofname_mtl, float scale){
  scene.set_output_fname(ofname_obj, ofname_mtl, scale);
}
void Application::set_command_iterations(char com, int ite){
  command=com;
  iterations=ite;
  scene.set_command_iterations(com,ite);
}  

void Application::init() {
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(0,0,0,0);
}

void Application::keyboardFunc(unsigned char key, int x, int y) {
    switch (key) {
    case 'x':
    case 'X':
        // added by T.Narumi
        scene.output_obj();
        glutLeaveMainLoop();
        break;
    case 'o':
    case 'O':
        scene.calculateFormFactors();
        /* reset viewport/projection that were modified while calculating form factors */
        reshapeFunc(width, height);
        /* start calculating radiosities */
        isSimulating = true;
        break;
    // added by T.Narumi
    case 'g':
    case 'G':
        scene.calculateFormFactors();
        /* reset viewport/projection that were modified while calculating form factors */
        reshapeFunc(width, height);
        /* start calculating radiosities */
        isSimulating = true;
	isGather = true;
        break;
    case 's':
    case 'S':
        scene.calculateAreas();
	scene.initializeUnshotRadiosities();
	//scene.calculateFormFactors();
        /* reset viewport/projection that were modified while calculating form factors */
        reshapeFunc(width, height);
        /* start calculating radiosities */
        isSimulating = true;
	isProgressive = true;
        break;
    case 'p':
    case 'P':
        perspective = !perspective;
        reshapeFunc(width, height);
        glutPostRedisplay();
        break;
    case 'r':
    case 'R':
        scene.calculateRadiosities();
        scene.interpolateColors();
        numIterations++;
        glutPostRedisplay();
        break;
    }
}

void Application::reshapeFunc(int width, int height) {
    this->width = width;
    this->height = height;
    const float aspectRatio = (float) width / (float) height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (perspective) {
        gluPerspective(75, aspectRatio, 0.1, 10000);
    } else {
        glOrtho(-24, 24, -24, 24, 0.01, 10000);
    }

    glMatrixMode(GL_MODELVIEW);
}

void Application::displayFunc() {
    if (!isSimulating) {
        greetingMessage();
        glFinish();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (perspective) {
        camera.applyTransformation();
    } else {
        glTranslatef(0, -14, -50);
        glRotatef(-camera.pitch, 1, 0, 0);
        glRotatef(-camera.yaw, 0, 1, 0);
    }

    // added by T.Narumi
    glScalef(10, 10, 10);
    scene.render();
    glFinish();
}

void Application::idleFunc() {
    if (isSimulating && numIterations < MAX_ITERATIONS) {
      // added by T.Narumi
      if(isGather)
	scene.calculateGatherRadiosities();
      else if(isProgressive)
	scene.calculateProgressiveRadiosities();
      else
        scene.calculateRadiosities();
      scene.interpolateColors();
      numIterations++;
      glutPostRedisplay();
    }

    // added by T.Narumi
    if(command!='\0'){
      int ite=0;
      if(command=='g' || command=='o'){
        scene.calculateFormFactors();
      } else if(command=='s'){
        scene.calculateAreas();
	scene.initializeUnshotRadiosities();
      }
      while(ite<iterations){
	if(command=='o')      scene.calculateRadiosities();
	else if(command=='g') scene.calculateGatherRadiosities();
	else if(command=='s') scene.calculateProgressiveRadiosities();
	ite++;
      }
      scene.output_obj();
      glutLeaveMainLoop();
    }
}

bool mouseMoving = false;
int px, py;
void Application::mouseFunc(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        mouseMoving = true;
        px = x;
        py = y;
    } else {
        mouseMoving = false;
    }
}

void Application::mouseMotionFunc(int x,int y) {
    if (mouseMoving) {
		camera.yaw += MOUSE_SENSITIVITY * (px - x);
		camera.pitch += MOUSE_SENSITIVITY * (py -y);

		if (camera.yaw > 360) {
			camera.yaw -= 360;
		} else if (camera.yaw < 0) {
			camera.yaw += 360;
		}
		if (camera.pitch > 360) {
			camera.pitch -= 360;
		} else if (camera.pitch < 0) {
			camera.pitch += 360;
		}

		px = x;
		py = y;
		glutPostRedisplay();
	}
}

void Application::greetingMessage() {
    char message[256] =
    "Press O/G/S to begin simulation.\n"\
    "Press X to exit when you are done viewing the image.\n\n"\
    "Press P to switch between perspective/orthographic \nviews.\n"\
    "Click and drag mouse to rotate the view.";

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(-width/2,width/2,-height/2,height/2);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,1);
    glRasterPos3f(-width/2, 0, 0);
    glutBitmapString(GLUT_BITMAP_HELVETICA_12, (const unsigned char*) message);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
