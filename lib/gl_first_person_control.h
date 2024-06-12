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
float jumpVelocity = 4;

float gravityY = -9.81;
float friction = 0.8, frictionAir = 0.2;

bool flying = true, running = false, spaceKeyPress = false;
bool firstPersonPause = false;
struct timespec spaceKeyInterval;

int windowCenterX = -1, windowCenterY = -1;
Vector3f cameraPos = {0.0, 0.0, 0.0};
Vector3f cameraVelocity = {0.0, 0.0, 0.0};
Vector3f cameraAngle = {0.0, 0.0, 0.0};
Vector3f cameraVec = {0.0, 0.0, 0.0};

int lastMouseX = -1, lastMouseY = -1;

void firstPersonMouseReset() {
    lastMouseX = windowCenterX;
    lastMouseY = windowCenterY;
    glutWarpPointer(lastMouseX, lastMouseY);
}

void firstPersonMouse(int x, int y) {
    if (firstPersonPause) return;

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
    vx(cameraAngle) += (float)-deltaY * mouseSensitivity * deltaTimeUpdate;
    if (vx(cameraAngle) > 89.9999)
        vx(cameraAngle) = 89.9999;
    else if (vx(cameraAngle) < -89.9999)
        vx(cameraAngle) = -89.9999;
    vy(cameraAngle) += (float)deltaX * mouseSensitivity * deltaTimeUpdate;
    if (vx(cameraAngle) > 360)
        vx(cameraAngle) -= 360;
    else if (vx(cameraAngle) < -360)
        vx(cameraAngle) += 360;

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
}

void calculateCameraMovement() {
    double cameraAngley = vy(cameraAngle) * M_Ang2Rad;
    double cameraAnglex = vx(cameraAngle) * M_Ang2Rad;
    vx(cameraVec) = cos(cameraAngley) * cos(cameraAnglex);
    vy(cameraVec) = sin(cameraAnglex);
    vz(cameraVec) = sin(cameraAngley) * cos(cameraAnglex);

    if (firstPersonPause) return;

    // XZ friction
    Vector3f v = vec3Clone(cameraVelocity);
    vy(v) = 0;
    float speed = vec3fLength(v);
    if (speed > 0) {
        float n = 1 * -gravityY;
        float f = (flying ? frictionAir : friction) * n * deltaTimeUpdate;

        Vector3f frictionAcc = {0, 0, 0};
        vec3fMinus(v, frictionAcc);
        if (speed < f) {
            vx(cameraVelocity) = 0;
            vz(cameraVelocity) = 0;
        } else {
            vec3fScale(f, frictionAcc);
            vec3fAdd(frictionAcc, cameraVelocity);
        }
    }

    // XZ movement
    bool anyKey = false;
    Vector3f moveAccXZ = {0, 0, 0};
    if (keys[GLUT_KEY_UP]) {
        vec3fAdd((Vector3f){vx(cameraVec), 0, vz(cameraVec)}, moveAccXZ);
        anyKey = true;
    }
    if (keys[GLUT_KEY_DOWN]) {
        vec3fAdd((Vector3f){-vx(cameraVec), 0, -vz(cameraVec)}, moveAccXZ);
        anyKey = true;
    }
    if (keys[GLUT_KEY_RIGHT]) {
        Vector3f right;
        vec3fCross((Vector3f){vx(cameraVec), 0, vz(cameraVec)}, (Vector3f){0, 1, 0}, right);
        vec3fAdd(right, moveAccXZ);
        anyKey = true;
    }
    if (keys[GLUT_KEY_LEFT]) {
        Vector3f left;
        vec3fCross((Vector3f){vx(cameraVec), 0, vz(cameraVec)}, (Vector3f){0, -1, 0}, left);
        vec3fAdd(left, moveAccXZ);
        anyKey = true;
    }
    if (anyKey) {
        vec3fNormalize(moveAccXZ);
        vec3fScale((flying ? flyAcc : moveAcc) * deltaTimeUpdate, moveAccXZ);
        vec3fAdd(moveAccXZ, cameraVelocity);
        // Limit speed
        vec3fSet(v, cameraVelocity);
        vy(v) = 0;
        float speed = vec3fLength(v);
        float maxSpeed = (flying ? flySpeed : (running ? runningSpeed : moveSpeed));
        if (speed > maxSpeed) {
            vec3fNormalize(v);
            vec3fScale(maxSpeed, v);
            vx(cameraVelocity) = vx(v);
            vz(cameraVelocity) = vz(v);
        }
        // printf("%f\n",vec3fLength(v));
    }

    // Y movement
    if (flying) {
        anyKey = false;
        if (keys[' '] && !keys[GLUT_KEY_LEFTSHIFT]) {
            vy(cameraVelocity) = moveSpeed;
            anyKey = true;
        }
        if (keys[GLUT_KEY_LEFTSHIFT] && !keys[' ']) {
            vy(cameraVelocity) = -moveSpeed;
            anyKey = true;
        }
        if (!anyKey)
            vy(cameraVelocity) = 0;
    } else {
        vy(cameraVelocity) += gravityY * deltaTimeUpdate;
        if (keys[' '] && vy(cameraPos) < 2.0001)
            vy(cameraVelocity) = jumpVelocity;
        running = keys[GLUT_KEY_LEFTSHIFT];
    }
    vec3fSet(v, cameraVelocity);
    vec3fScale(deltaTimeUpdate, v);
    vec3fAdd(v, cameraPos);
    // Ground
    if (vy(cameraPos) < 2) {
        vy(cameraPos) = 2;
        vy(cameraVelocity) = 0;
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
    // glTranslatef(vx(cameraPos), vy(cameraPos), vz(cameraPos));
    // glRotatef(vy(cameraAngle), 0, 1, 0);
    // glRotatef(vx(cameraAngle), 0, 1, 0);
    gluLookAt(vx(cameraPos), vy(cameraPos), vz(cameraPos),
              vx(cameraPos) + vx(cameraVec), vy(cameraPos) + vy(cameraVec), vz(cameraPos) + vz(cameraVec),
              0, 1, 0);  // up direction

    // printf("%f, %f, %f\n", vx(cameraVec), vy(cameraVec), vz(cameraVec));
}

#endif