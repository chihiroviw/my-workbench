#ifndef CAMERA_H
#define CAMERA_H

#include <GL/freeglut.h>
#include "vec3.h"

struct Camera {
    vec3 position;
    float pitch;
    float yaw;

    /* apply inverse of camera transform to the scene */
    void applyTransformation ()  {
        glRotatef(-pitch, 1, 0, 0);
        glRotatef(-yaw, 0, 1, 0);
        glTranslatef(-position.x, -position.y, -position.z);
    }
};

#endif // CAMERA_H
