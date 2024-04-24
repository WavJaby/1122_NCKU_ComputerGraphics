#include <GL/glut.h>

GLuint debugGridCreate(int gridSize) {
    GLuint xzGridList = glGenLists(2);
    glNewList(xzGridList, GL_COMPILE);
    glDisable(GL_LIGHTING);
    glColor4f(1, 0, 0, 0.8);
    glBegin(GL_LINES);
    for (int i = -gridSize; i <= gridSize; ++i) {
        glVertex3f(-gridSize, 0, i);
        glVertex3f(gridSize, 0, i);
    }
    glEnd();
    glEndList();

    glNewList(xzGridList + 1, GL_COMPILE);
    glColor4f(0, 0, 1, 0.8);
    glBegin(GL_LINES);
    for (int i = -gridSize; i <= gridSize; ++i) {
        glVertex3f(i, 0, -gridSize);
        glVertex3f(i, 0, gridSize);
    }
    glEnd();
    glEndList();
    return xzGridList;
}

static inline void debugGridRender(GLuint id) {
    glCallList(id);
    glCallList(id + 1);
}