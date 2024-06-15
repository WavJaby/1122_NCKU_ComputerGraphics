#ifndef __GLFW_CAMERA_H__
#define __GLFW_CAMERA_H__

#include "linmath.h"

vec3 cameraPos = {0.0, 0.0, 0.0};
vec3 cameraAngle = {0.0, 0.0, 0.0};
vec3 cameraVec = {0.0, 0.0, 0.0};
float cameraFov = 45;
mat4x4 cameraProjectionMat = mat4x4_identity_c, cameraViewMat = mat4x4_identity_c;

void camera_windowSizeUpdate(int width, int height) {
    mat4x4_perspective(cameraProjectionMat, cameraFov * M_DEG_2_RAD, (float)width / height, 0.01, 1000);
}

void camera_updateViewMatrix() {
    double a[3] = {cameraAngle[0] * M_DEG_2_RAD, cameraAngle[1] * M_DEG_2_RAD, cameraAngle[2] * M_DEG_2_RAD};
    double sina0 = sin(a[0]);
    double sina1 = sin(a[1]);
    double sina2 = sin(a[2]);
    double cosa1 = cos(a[1]);
    double cosa2 = cos(a[2]);
    cameraVec[0] = -cosa1 * sina0 * sina2 - sina1 * cosa2;
    cameraVec[1] = cos(a[0]) * sina2;
    cameraVec[2] = -sina1 * sina0 * sina2 + cosa1 * cosa2;

    mat4x4 rotX, rotY, trans;
    mat4x4_rotate_create_X(rotX, a[0]);
    mat4x4_rotate_create_Y(rotY, a[1]);
    mat4x4_translate_create(trans, cameraPos[0], cameraPos[1], cameraPos[2]);
    mat4x4_mul(cameraViewMat, mat4x4_identity, rotX);
    mat4x4_mul(cameraViewMat, cameraViewMat, rotY);
    mat4x4_mul(cameraViewMat, cameraViewMat, trans);

    // mat4x4_look_at(cameraViewMat, cameraPos, (vec3){0, 0, 0}, (vec3){0, 1, 0});

    // printf("%f, %f, %f\n", vx(cameraVec), vy(cameraVec), vz(cameraVec));
}

#endif