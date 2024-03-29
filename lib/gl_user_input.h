#ifndef __GL_USER_INPUT_H__
#define __GL_USER_INPUT_H__

#include <GL/glut.h>

// #define DEBUGKEY
#define GLUT_KEY_ESC 0x001B
#define GLUT_KEY_LEFTSHIFT 0x0070
#define GLUT_KEY_RIGHTSHIFT 0x0071
#define GLUT_KEY_BACKSPACE 0x006D
#define GLUT_KEY_TAB 0x006E
#define GLUT_KEY_ENTER 0x00D

char keys[127] = {0};
char keysState[127] = {0};
char keysOnPress[127] = {0};

static inline unsigned char keyMapping(unsigned char key) {
    switch (key) {
        case '!':
            return '1';
        case '@':
            return '2';
        case '#':
            return '3';
        case '$':
            return '4';
        case '%':
            return '5';
        case '^':
            return '6';
        case '&':
            return '7';
        case '*':
            return '8';
        case '(':
            return '9';
        case ')':
            return '0';
        case '_':
            return '-';
        case '+':
            return '=';
        case '{':
            return '[';
        case '}':
            return ']';
        case '|':
            return '\\';
        case ':':
            return ';';
        case '"':
            return '\'';
        case '~':
            return '`';
        case '<':
            return ',';
        case '>':
            return '.';
        case '?':
            return '/';
        case '\x8':
            return GLUT_KEY_BACKSPACE;
        case '\x9':
            return GLUT_KEY_TAB;
        default:
            if (key >= 'a' && key <= 'z')
                return key - 32;
    }
    return key;
}

void keyDown(unsigned char key, int x, int y) {
    keys[keyMapping(key)] = 1;
#ifdef DEBUGKEY
    printf(">%d\n", keyMapping(key));
#endif
}

void keyUp(unsigned char key, int x, int y) {
    keys[keyMapping(key)] = 0;
#ifdef DEBUGKEY
    printf(" %d\n", keyMapping(key));
#endif
}

void specialKeyDown(int key, int x, int y) {
    keys[key] = 1;
#ifdef DEBUGKEY
    printf("-%d\n", key);
#endif
}

void specialKeyUp(int key, int x, int y) {
    keys[key] = 0;
#ifdef DEBUGKEY
    printf(" %d\n", key);
#endif
}

void userInputInitUpdate() {
    for (size_t i = 0; i < 127; i++) {
        if (keys[i])
            keysState[i] = 1;
        else if (keysState[i]) {
            keysOnPress[i] = 1;
            keysState[i] = 0;
        } else
            keysOnPress[i] = 0;
    }
}

void userInputInit() {
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialKeyDown);
    glutSpecialUpFunc(specialKeyUp);
}

void userInputMouseFunc(void (*callback)(int, int)) {
    glutPassiveMotionFunc(callback);
    glutMotionFunc(callback);
}

#endif