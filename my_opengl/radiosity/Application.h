#ifndef APPLICATION_H
#define APPLICATION_H

#include <GL/freeglut.h>
#include "Camera.h"
#include "Scene.h"

#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 700
#define MOUSE_SENSITIVITY 0.2

#define MAX_ITERATIONS 30

class Application {
public:
    Application();
  
    // added by T.Narumi
    Application(const char *fname);
    void set_output_fname(const char *ofname_obj, const char *ofname_mtl, float scale);
    void set_command_iterations(char com, int ite);
  
    void init();

    void keyboardFunc(unsigned char key, int x, int y);
    void keyboardUpFunc(unsigned char key, int x, int y);
    void mouseMotionFunc(int x,int y);
    void mouseFunc(int button, int state, int x, int y);
    void reshapeFunc(int w, int h);
    void displayFunc();
    void idleFunc();

    void greetingMessage ();

protected:
    Camera camera;
    bool perspective;
    int width, height;

    Scene scene;

    bool isSimulating;
    int numIterations;

    // added by T.Narumi
    bool isGather;
    bool isProgressive;
    char command='\0';
    int iterations=1;
};

#endif // APPLICATION_H
