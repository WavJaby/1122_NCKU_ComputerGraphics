#ifndef __GL_FIRST_PERSION_CONTROL_H__
#define __GL_FIRST_PERSION_CONTROL_H__

#include <GL/glut.h>
#include <stdbool.h>

#include "fps_counter.h"
#include "gl_user_input.h"
#include "gl_vector.h"

#define M_PI 3.14159265358979323846
#define M_Ang2Rad 0.01745329251

#define FLY_TOGGLE_INTERVAL 300

float mouseSensitivity = 1.5;
float flySpeed = 7, flyAcc = 40;
float moveSpeed = 3, runningSpeed = 5, moveAcc = 50;

float jumpVelocity = 3;
float gravityY = -9.81;
float friction = 0.8, frictionAir = 0.2;
bool flying = true, running = false, spaceKeyPress = false;
struct timespec spaceKeyInterval;

int windowCenterX = -1, windowCenterY = -1;
GLVector3f cameraPos = {0.0, 0.0, 0.0};
GLVector3f cameraVelocity = {0.0, 0.0, 0.0};
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

    // XZ friction
    GLVector3f v = cameraVelocity;
    v.y = 0;
    float speed = GLVector3Length(v);
    if (speed > 0) {
        float n = 1 * -gravityY;
        float f = (flying ? frictionAir : friction) * n * deltaTimeTick;

        GLVector3f frictionAcc = (GLVector3f){0, 0, 0};
        GLVector3MinusTo(v, &frictionAcc);
        if (speed < f) {
            cameraVelocity.x = 0;
            cameraVelocity.z = 0;
        } else {
            GLVector3ScaleTo(f, &frictionAcc);
            GLVector3AddTo(frictionAcc, &cameraVelocity);
        }
    }

    // XZ movement
    bool anyKey = false;
    GLVector3f moveAccXZ = {0, 0, 0};
    if (keys['W']) {
        GLVector3AddTo((GLVector3f){cameraVec.x, 0, cameraVec.z}, &moveAccXZ);
        anyKey = true;
    }
    if (keys['S']) {
        GLVector3AddTo((GLVector3f){-cameraVec.x, 0, -cameraVec.z}, &moveAccXZ);
        anyKey = true;
    }
    if (keys['D']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, 1, 0});
        GLVector3AddTo(right, &moveAccXZ);
        anyKey = true;
    }
    if (keys['A']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, -1, 0});
        GLVector3AddTo(right, &moveAccXZ);
        anyKey = true;
    }
    if (anyKey) {
        GLVector3NormalizeTo(&moveAccXZ);
        GLVector3ScaleTo((flying ? flyAcc : moveAcc) * deltaTimeTick, &moveAccXZ);
        GLVector3AddTo(moveAccXZ, &cameraVelocity);
        // Limit speed
        v = cameraVelocity;
        v.y = 0;
        float speed = GLVector3Length(v);
        float maxSpeed = (flying ? flySpeed : (running ? runningSpeed : moveSpeed));
        if (speed > maxSpeed) {
            GLVector3NormalizeTo(&v);
            GLVector3ScaleTo(maxSpeed, &v);
            cameraVelocity.x = v.x;
            cameraVelocity.z = v.z;
        }
        // printf("%f\n",GLVector3Length(v));
    }

    // Y movement
    if (flying) {
        anyKey = false;
        if (keys[' '] && !keys[GLUT_KEY_LEFTSHIFT]) {
            cameraVelocity.y = moveSpeed;
            anyKey = true;
        }
        if (keys[GLUT_KEY_LEFTSHIFT] && !keys[' ']) {
            cameraVelocity.y = -moveSpeed;
            anyKey = true;
        }
        if (!anyKey)
            cameraVelocity.y = 0;
    } else {
        cameraVelocity.y += gravityY * deltaTimeTick;
        if (keys[' '] && cameraPos.y < 1.0001)
            cameraVelocity.y = jumpVelocity;
        running = keys[GLUT_KEY_LEFTSHIFT];
    }
    v = cameraVelocity;
    GLVector3ScaleTo(deltaTimeTick, &v);
    GLVector3AddTo(v, &cameraPos);
    // Ground
    if (cameraPos.y < 1) {
        cameraPos.y = 1;
        cameraVelocity.y = 0;
        if (flying)
            flying = false;
    }

    // Fly mode toggle
    if (!keys[' '])
        spaceKeyPress = false;
    else if (!spaceKeyPress) {
        spaceKeyPress = true;
        uint64_t interval = getTimePass(&spaceKeyInterval) / 1000;
        // Toggle fly
        if (interval < FLY_TOGGLE_INTERVAL)
            flying = !flying;
    }
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