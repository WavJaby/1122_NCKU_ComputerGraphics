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
    if (lastMouseX == -1) {
        firstPersonMouseReset();
        // lastMouseX = x;
        // lastMouseY = y;
    }
    if (lastMouseX == x && lastMouseY == y)
        return;

    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;
    cameraAngle.x += (float)-deltaY * mouseSensitivity;
    if (cameraAngle.x > 89.9999)
        cameraAngle.x = 89.9999;
    else if (cameraAngle.x < -89.9999)
        cameraAngle.x = -89.9999;
    cameraAngle.y += (float)deltaX * mouseSensitivity;
    if (cameraAngle.x > 360)
        cameraAngle.x -= 360;
    else if (cameraAngle.x < -360)
        cameraAngle.x += 360;

    // if (abs(lastMouseX - windowCenterX) > 5 || abs(lastMouseY - windowCenterY) > 5) {
    //     firstPersonMouseReset();
    // }
}

void firstPersonInit() {
    glutSetCursor(GLUT_CURSOR_NONE);
    // firstPersonMouseReset();
    userInputMouseFunc(firstPersonMouse);
}

void firstPersonWindowSizeUpdate(int width, int height) {
    windowCenterX = width >> 1;
    windowCenterY = height >> 1;
}

void calculateCameraMovement() {
    double cameraAngley = cameraAngle.y * M_Ang2Rad;
    double cameraAnglex = cameraAngle.x * M_Ang2Rad;
    cameraVec.x = cos(cameraAngley) * cos(cameraAnglex);
    cameraVec.y = sin(cameraAnglex);
    cameraVec.z = sin(cameraAngley) * cos(cameraAnglex);

    glLoadIdentity();
    // glTranslatef(cameraPos.x, cameraPos.y, cameraPos.z);
    // glRotatef(cameraAngle.y, 0, 1, 0);
    // glRotatef(cameraAngle.x, 0, 1, 0);
    gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
              cameraPos.x + cameraVec.x, cameraPos.y + cameraVec.y, cameraPos.z + cameraVec.z,
              0, 1, 0);  // up direction

    // printf("%f, %f, %f\n", cameraVec.x, cameraVec.y, cameraVec.z);

    GLVector3f move = {0, 0, 0};
    if (keys['w'])
        move = (GLVector3f){cameraVec.x, 0, cameraVec.z};
    else if (keys['s'])
        move = (GLVector3f){-cameraVec.x, 0, -cameraVec.z};

    if (keys['d']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, 1, 0});
        GLVector3AddTo(right, &move);
    } else if (keys['a']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, -1, 0});
        GLVector3AddTo(right, &move);
    }
    GLVector3NormalizeTo(&move);
    GLVector3ScaleTo(1.0, &move);
    GLVector3AddTo(move, &cameraPos);
}

#endif