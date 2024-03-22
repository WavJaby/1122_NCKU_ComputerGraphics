#ifndef __GL_USER_INPUT_H__
#define __GL_USER_INPUT_H__

#include <GL/glut.h>
char keys[255] = {0};

void keyDown(unsigned char key, int x, int y) {
    keys[key] = 1;
}

void keyUp(unsigned char key, int x, int y) {
    keys[key] = 0;
}

void userInputInit() {
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
}

void userInputMouseFunc(void (*callback)(int, int)) {
    glutPassiveMotionFunc(callback);
    glutMotionFunc(callback);
}

#endif