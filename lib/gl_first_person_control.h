#ifndef __GL_FIRST_PERSION_CONTROL_H__
#define __GL_FIRST_PERSION_CONTROL_H__

#include <GL/glut.h>
#include <stdbool.h>

#include "fps_counter.h"
#include "gl_user_input.h"
#include "gl_vector.h"

#define M_PI 3.14159265358979323846
#define M_Ang2Rad 0.01745329251

#define FLY_TOGGLE_INTERVAL 200

float mouseSensitivity = 1.5;
float moveSpeed = 2;

float gravityY = -9.81;
float friction = 0.8;
bool flying = true, spaceKeyPress = false;
struct timespec spaceKeyInterval;

int windowCenterX = -1, windowCenterY = -1;
GLVector3f cameraPos = {0.0, 0.0, 0.0};
GLVector3f cameraVolcity = {0.0, 0.0, 0.0};
GLVector3f cameraAngle = {0.0, 0.0, 0.0};
GLVector3f cameraVec = {0.0, 0.0, 0.0};

int lastMouseX = -1, lastMouseY = -1;

void firstPersonMouseReset() {
    lastMouseX = windowCenterX;
    lastMouseY = windowCenterY;
    glutWarpPointer(lastMouseX, lastMouseY);
}

void firstPersonMouse(int x, int y) {
    // printf("%d, %d\n", x, y);
    if (lastMouseX == -1 && (x || y)) {
        lastMouseX = x;
        lastMouseY = y;
        printf("Mouse init\n");
        return;
    }
    if (lastMouseX == x && lastMouseY == y)
        return;

    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;
    cameraAngle.x += (float)-deltaY * mouseSensitivity * deltaTimeTick;
    if (cameraAngle.x > 89.9999)
        cameraAngle.x = 89.9999;
    else if (cameraAngle.x < -89.9999)
        cameraAngle.x = -89.9999;
    cameraAngle.y += (float)deltaX * mouseSensitivity * deltaTimeTick;
    if (cameraAngle.x > 360)
        cameraAngle.x -= 360;
    else if (cameraAngle.x < -360)
        cameraAngle.x += 360;

    if (abs(lastMouseX - windowCenterX) > 5 || abs(lastMouseY - windowCenterY) > 5) {
        firstPersonMouseReset();
    }
}

void firstPersonInit() {
    glutSetCursor(GLUT_CURSOR_NONE);
    userInputMouseFunc(firstPersonMouse);
    getTimePass(&spaceKeyInterval);
}

void firstPersonWindowSizeUpdate(int width, int height) {
    windowCenterX = width >> 1;
    windowCenterY = height >> 1;
    printf("WindowSize: %dx%d\n", width, height);
}

void calculateCameraMovement() {
    double cameraAngley = cameraAngle.y * M_Ang2Rad;
    double cameraAnglex = cameraAngle.x * M_Ang2Rad;
    cameraVec.x = cos(cameraAngley) * cos(cameraAnglex);
    cameraVec.y = sin(cameraAnglex);
    cameraVec.z = sin(cameraAngley) * cos(cameraAnglex);

    bool anyKey = false;
    GLVector3f move = {0, 0, 0};
    if (keys['W']) {
        GLVector3AddTo((GLVector3f){cameraVec.x, 0, cameraVec.z}, &move);
        anyKey = true;
    }
    if (keys['S']) {
        GLVector3AddTo((GLVector3f){-cameraVec.x, 0, -cameraVec.z}, &move);
        anyKey = true;
    }
    if (keys['D']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, 1, 0});
        GLVector3AddTo(right, &move);
        anyKey = true;
    } else if (keys['A']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, -1, 0});
        GLVector3AddTo(right, &move);
        anyKey = true;
    }

    if (anyKey) {
        GLVector3NormalizeTo(&move);
        GLVector3ScaleTo(moveSpeed * deltaTimeTick, &move);
        GLVector3AddTo(move, &cameraVolcity);
    }

    GLVector3f v = cameraVolcity;
    v.y = 0;
    float speed = GLVector3Length(v);
    if (speed > 0) {
        GLVector3f frictionAcc = (GLVector3f){0, 0, 0};
        GLVector3MinusTo(v, &frictionAcc);
        GLVector3ScaleTo(fmin(speed, friction), &frictionAcc);
        GLVector3AddTo(frictionAcc, &cameraVolcity);
        printf("%f\n", speed);
    }

    // Y
    if (flying) {
        if (keys[' ']) {
            GLVector3AddTo((GLVector3f){0, moveSpeed * deltaTimeTick, 0}, &cameraVolcity);
        }
        if (keys[GLUT_KEY_LEFTSHIFT]) {
            GLVector3AddTo((GLVector3f){0, -moveSpeed * deltaTimeTick, 0}, &cameraVolcity);
        }
    } else {
        // velocityY += gravityY * deltaTimeTick;
        // cameraPos.y += velocityY * deltaTimeTick;
        if (cameraPos.y < 1)
            cameraPos.y = 1;
    }
    if (!keys[' '])
        spaceKeyPress = false;
    else if (!spaceKeyPress) {
        spaceKeyPress = true;
        uint64_t interval = getTimePass(&spaceKeyInterval) / 1000;
        // Toggle fly
        if (interval < FLY_TOGGLE_INTERVAL)
            flying = !flying;
    }
    GLVector3AddTo(cameraVolcity, &cameraPos);
}

void calculateCameraMatrix() {
    glLoadIdentity();
    // glTranslatef(cameraPos.x, cameraPos.y, cameraPos.z);
    // glRotatef(cameraAngle.y, 0, 1, 0);
    // glRotatef(cameraAngle.x, 0, 1, 0);
    gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
              cameraPos.x + cameraVec.x, cameraPos.y + cameraVec.y, cameraPos.z + cameraVec.z,
              0, 1, 0);  // up direction

    // printf("%f, %f, %f\n", cameraVec.x, cameraVec.y, cameraVec.z);
}

#endif