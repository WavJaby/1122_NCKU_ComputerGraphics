#ifndef __GLFW_CAMERA_H__
#define __GLFW_CAMERA_H__

#include "linmath.h"

vec3 cameraPos = {0.0, 0.0, 0.0};
vec3 cameraAngle = {0.0, 0.0, 0.0};
vec3 cameraVec = {0.0, 0.0, 0.0};
mat4x4 cameraProjectionMat = mat4x4_identity_c, cameraViewMat = mat4x4_identity_c;

void camera_windowSizeUpdate(int width, int height) {
    mat4x4_perspective(cameraProjectionMat, 1.57, (float)width / height, 0.01, 1000);
}

void camera_updateViewMatrix() {
    mat4x4 rotX, rotY, trans;
    mat4x4_rotate_create_X(rotX, cameraAngle[0] * (float)M_PI / 180.0f);
    mat4x4_rotate_create_Y(rotY, cameraAngle[1] * (float)M_PI / 180.0f);
    mat4x4_translate_create(trans, cameraPos[0], cameraPos[1], cameraPos[2]);
    mat4x4_mul(cameraViewMat, mat4x4_identity, rotX);
    mat4x4_mul(cameraViewMat, cameraViewMat, rotY);
    mat4x4_mul(cameraViewMat, cameraViewMat, trans);

    // printf("%f, %f, %f\n", vx(cameraVec), vy(cameraVec), vz(cameraVec));
}

#endif