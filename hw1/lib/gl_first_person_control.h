#ifndef __GL_FIRST_PERSION_CONTROL_H__
#define __GL_FIRST_PERSION_CONTROL_H__

#include <GL/glut.h>

#include "gl_user_input.h"
#include "gl_vector.h"

#define M_PI 3.14159265358979323846
#define M_Ang2Rad 0.01745329251

int windowCenterX = 0, windowCenterY = 0;
GLVector3f cameraPos = {0.0, 0.0, 0.0};
GLVector3f cameraAngle = {0.0, 0.0, 0.0};
GLVector3f cameraVec = {0.0, 0.0, 0.0};
float mouseSensitivity = 0.2;

int lastMouseX = -1, lastMouseY = -1;

void firstPersonMouseReset() {
    lastMouseX = windowCenterX;
    lastMouseY = windowCenterY;
    glutWarpPointer(lastMouseX, lastMouseY);
}

void firstPersonMouse(int x, int y) {
    // printf("%d, %d\n", x, y);
    if (lastMouseX == -1)
        firstPersonMouseReset();
    if (lastMouseX == x && lastMouseY == y)
        return;

    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;
    cameraAngle.x += (float)-deltaY * M_Ang2Rad * mouseSensitivity;
    cameraAngle.y += (float)deltaX * M_Ang2Rad * mouseSensitivity;

    if (abs(lastMouseX - windowCenterX) > 5 || abs(lastMouseY - windowCenterY) > 5) {
        firstPersonMouseReset();
    }
}

void firstPersonInit() {
    firstPersonMouseReset();
    userInputMouseFunc(firstPersonMouse);
}

void firstPersonWindowSizeUpdate(int width, int height) {
    windowCenterX = width >> 1;
    windowCenterY = height >> 1;
}

void calculateCameraDiraction() {
    cameraVec.x = cos(cameraAngle.y) * cos(cameraAngle.x);
    cameraVec.y = sin(cameraAngle.x);
    cameraVec.z = sin(cameraAngle.y) * cos(cameraAngle.x);
    gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
              cameraPos.x + cameraVec.x, cameraPos.y + cameraVec.y, cameraPos.z + cameraVec.z,
              0, 1, 0);  // up direction
}

#endif