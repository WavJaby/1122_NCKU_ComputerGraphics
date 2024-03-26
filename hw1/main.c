#include <GL/glut.h>
#include <stdio.h>

#include "lib/fps_counter.h"
#include "lib/gl_first_person_control.h"
#include "lib/gl_user_input.h"
#include "lib/gl_vector.h"
#include "stl_reader.h"

char title[32] = "F74114760 hw1";
int refreshMills = 1000 / 100;  // refresh interval in milliseconds

int nTriangles = 0;
STriangle* triangles = 0;
GLuint displayList;
GLfloat angle = 0;

void fpsUpdate(float fps) {
    char cBuffer[64];
    sprintf(cBuffer, "%s fps: %.2f", title, fps);
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

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat ambientLight[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat diffuseLight[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat specularLight[] = {1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    GLfloat lightPosition[] = {0.0, 1.0, 0.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, specularLight);

    triangles = loadStlBinary("Bunny_Binary.stl", &nTriangles);
    // loadStlASCII("Bunny_ASCII.stl", &nTriangles);
    printf("%d triangles loaded\n", nTriangles);

    // Display List
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < nTriangles; ++i) {
        glNormal3fv(triangles[i].normal);
        glVertex3fv(triangles[i].a);
        glVertex3fv(triangles[i].b);
        glVertex3fv(triangles[i].c);
    }
    glEnd();
    glEndList();

    cameraPos = (GLVector3f){0, 0, -400};
    cameraAngle = (GLVector3f){0, 90, 0};
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);                          // To operate on model-view matrix

    calculateCameraMovement();

    if (keys['x'])
        angle += 2;
    if (keys['z'])
        angle -= 2;

    glRotatef(-90, 1, 0, 0);
    glRotatef(angle, 0, 1, 0);
    glCallList(displayList);

    frameUpdate(fpsUpdate);
    glutSwapBuffers();
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

void timer(int value) {
    glutPostRedisplay();  // Post re-paint request to activate display()
    if (keys['\033']) {
        glutDestroyWindow(glutGetWindow());
        return;
    }
    glutTimerFunc(refreshMills, timer, 0);  // next timer call milliseconds later
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);                                      // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);  // Enable double buffered mode
    glutInitWindowSize(1280, 720);                              // Set the window's initial width & height
    glutInitWindowPosition(50, 50);                             // Position the window's initial top-left corner
    glutCreateWindow(title);                                    // Create window with the given title
    glutDisplayFunc(display);                                   // Register callback handler for window re-paint event
    glutReshapeFunc(reshape);                                   // Register callback handler for window re-size event
    userInputInit();
    firstPersonInit();
    initGL();                    // OpenGL initialization
    glutTimerFunc(0, timer, 0);  // First timer call immediately
    glutMainLoop();              // Enter the infinite event-processing loop
    return 0;
}