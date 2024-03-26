#include <GL/glut.h>
#include <stdio.h>

#include "lib/fps_counter.h"
#include "lib/gl_first_person_control.h"
#include "lib/gl_user_input.h"
#include "lib/gl_vector.h"
#include "stl_reader.h"

char title[32] = "F74114760 hw1";
const int tickMills = 1000 / 60;
int refreshMills = 1000 / 100;  // refresh interval in milliseconds

STrianglesInfo triInfo;
GLuint displayList, gridListX, gridListZ;
GLfloat angle = 0;

void fpsUpdate(float fps, float tick) {
    char cBuffer[64];
    sprintf(cBuffer, "%s fps: %.2f, tick: %.2f", title, fps, tick);
    // printf("%s\n", cBuffer);
    glutSetWindowTitle(cBuffer);
}

/* Initialize OpenGL Graphics */
void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);               // Set background color to black and opaque
    glClearDepth(1.0f);                                 // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);                            // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);                             // Set the type of depth-test
    glShadeModel(GL_SMOOTH);                            // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    GLfloat ambientLight[] = {0.001, 0.001, 0.001, 1.0};
    GLfloat diffuseLight[] = {0.01, 0.01, 0.01, 1.0};
    GLfloat specularLight[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightPosition[] = {10.0, 10.0, 10.0, 1.0};
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, specularLight);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    loadStlASCII("Bunny_ASCII.stl", &triInfo);
    printf("%d triangles loaded\n", triInfo.trianglesCount);

    // Create model
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < triInfo.trianglesCount; ++i) {
        glNormal3fv(triInfo.triangles[i].normal);
        glVertex3f(
            triInfo.triangles[i].a[0] - triInfo.center.x,
            triInfo.triangles[i].a[1] - triInfo.center.y,
            triInfo.triangles[i].a[2] - triInfo.center.z);
        glVertex3f(
            triInfo.triangles[i].b[0] - triInfo.center.x,
            triInfo.triangles[i].b[1] - triInfo.center.y,
            triInfo.triangles[i].b[2] - triInfo.center.z);
        glVertex3f(
            triInfo.triangles[i].c[0] - triInfo.center.x,
            triInfo.triangles[i].c[1] - triInfo.center.y,
            triInfo.triangles[i].c[2] - triInfo.center.z);
    }
    glEnd();
    glEndList();

    int gridCount = 10;
    gridListX = glGenLists(1);
    glNewList(gridListX, GL_COMPILE);
    glBegin(GL_LINES);
    for (int i = -gridCount; i <= gridCount; ++i) {
        glVertex3f(-gridCount, 0, i);
        glVertex3f(gridCount, 0, i);
    }
    glEnd();
    glEndList();

    gridListZ = glGenLists(1);
    glNewList(gridListZ, GL_COMPILE);
    glBegin(GL_LINES);
    for (int i = -gridCount; i <= gridCount; ++i) {
        glVertex3f(i, 0, -gridCount);
        glVertex3f(i, 0, gridCount);
    }
    glEnd();
    glEndList();

    cameraPos = (GLVector3f){0, 1, -3};
    cameraAngle = (GLVector3f){0, 90, 0};
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);                          // To operate on model-view matrix

    calculateCameraMatrix();
    glPushMatrix();

    glEnable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glTranslatef(0, 0.5, 0);
    glRotatef(-90, 1, 0, 0);
    glRotatef(angle, 0, 1, 0);
    float scale = 1 / triInfo.maxSize;
    glScalef(scale, scale, scale);
    glCallList(displayList);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor4f(1, 0, 0, 0.8);
    glCallList(gridListX);
    glColor4f(0, 0, 1, 0.8);
    glCallList(gridListZ);

    frameUpdate();
    glutSwapBuffers();
}

void update() {
    if (keys['x'])
        angle += 2;
    if (keys['z'])
        angle -= 2;
    calculateCameraMovement();
    tickUpdate(fpsUpdate);
}

void reshape(GLsizei width, GLsizei height) {
    // Compute aspect ratio of the new window
    if (height == 0)
        height = 1;  // To prevent divide by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;
    firstPersonWindowSizeUpdate(width, height);

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping volume to match the viewport
    glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
    glLoadIdentity();             // Reset
    // Enable perspective projection with fovy, aspect, zNear and zFar
    gluPerspective(45.0f, aspect, 0.1f, 1000.0f);
}

struct timespec drawTime = {0};
void frameTimer(int value) {
    glutPostRedisplay();  // Post re-paint request to activate display()
    glutTimerFunc(refreshMills, frameTimer, 0);
}

void updateTimer(int value) {
    update();
    if (keys['\033']) {
        glutDestroyWindow(glutGetWindow());
        return;
    }
    glutTimerFunc(tickMills, updateTimer, 0);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);                                      // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);  // Enable double buffered mode
    glutInitWindowSize(1280, 720);                              // Set the window width & height
    glutInitWindowPosition(50, 50);                             // Position the window
    glutCreateWindow(title);                                    // Create window with title
    glutDisplayFunc(display);                                   // Register callback handler for window re-paint event
    glutReshapeFunc(reshape);                                   // Register callback handler for window re-size event
    userInputInit();
    firstPersonInit();
    initGL();
    glutTimerFunc(0, frameTimer, 0);
    glutTimerFunc(0, updateTimer, 0);
    glutMainLoop();
    return 0;
}