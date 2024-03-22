#include "main.h"

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#include "lib/gl_first_person_control.h"
#include "lib/gl_user_input.h"
#include "lib/gl_vector.h"

char title[32] = "3D Shape";
int refreshMills = 1000 / 60;  // refresh interval in milliseconds
GLfloat angleCube = 0.0f;      // Rotational angle for cube

void quad(GLVector3f* ver, GLVector3f normal, int a, int b, int c, int d) {
    glNormal3f(normal.x, normal.y, normal.z);
    glColor3f(1, 1, 1);
    // glColor3fv(color[a]);
    glVertex3f(ver[a].x, ver[a].y, ver[a].z);
    // glColor3fv(color[b]);
    glVertex3f(ver[b].x, ver[b].y, ver[b].z);
    // glColor3fv(color[c]);
    glVertex3f(ver[c].x, ver[c].y, ver[c].z);
    // glColor3fv(color[d]);
    glVertex3f(ver[d].x, ver[d].y, ver[d].z);
}

void colorcube() {
    GLVector3f ver[] = {
        {-1.0, -1.0, 1.0},
        {-1.0, 1.0, 1.0},
        {1.0, 1.0, 1.0},
        {1.0, -1.0, 1.0},
        {-1.0, -1.0, -1.0},
        {-1.0, 1.0, -1.0},
        {1.0, 1.0, -1.0},
        {1.0, -1.0, -1.0},
    };

    GLVector3f normal[] = {
        {0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0},
        {0.0, -1.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, -1.0},
        {-1.0, 0.0, 0.0},
    };

    glBegin(GL_QUADS);
    quad(ver, normal[0], 0, 3, 2, 1);
    quad(ver, normal[1], 2, 3, 7, 6);
    quad(ver, normal[2], 0, 4, 7, 3);
    quad(ver, normal[3], 1, 2, 6, 5);
    quad(ver, normal[4], 4, 5, 6, 7);
    quad(ver, normal[5], 0, 1, 5, 4);
    glEnd();
}

/* Initialize OpenGL Graphics */
void initGL() {
    glutSetCursor(GLUT_CURSOR_NONE);
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);               // Set background color to black and opaque
    glClearDepth(1.0f);                                 // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);                            // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);                             // Set the type of depth-test
    glShadeModel(GL_SMOOTH);                            // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
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
}

/* Handler for window-repaint event. Called back when the window first appears and
   whenever the window needs to be re-painted. */
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);                          // To operate on model-view matrix

    glLoadIdentity();
    calculateCameraDiraction();

    GLVector3f move = {0, 0, 0};
    if (keys['w'])
        move = (GLVector3f){cameraVec.x, 0, cameraVec.z};
    else if (keys['s'])
        move = (GLVector3f){-cameraVec.x, 0, -cameraVec.z};

    if (keys['d']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, 1, 0});
        GLVector3AddTo(right, &move);
    } else if (keys['a']) {
        GLVector3f right = GLVector3Cross((GLVector3f){cameraVec.x, 0, cameraVec.z}, (GLVector3f){0, -1, 0});
        GLVector3AddTo(right, &move);
    }
    GLVector3NormalizeTo(&move);
    GLVector3ScaleTo(0.3, &move);
    GLVector3AddTo(move, &cameraPos);

    printf("%f %f %f\n", move.x, move.y, move.z);

    // glTranslatef(2.0f, 0.0f, 0.0f);          // Move right and into the screen
    // glRotatef(angleCube, 1.0f, 1.0f, 1.0f);  // Rotate about (1,1,1)-axis
    colorcube();

    // glTranslatef(3.0f, 0.0f, 0.0f);          // Move right and into the screen
    // glRotatef(angleCube, 1.0f, 1.0f, 1.0f);  // Rotate about (1,1,1)-axis
    // colorcube();

    // // Render a pyramid consists of 4 triangles
    // glLoadIdentity();                           // Reset the model-view matrix
    // glTranslatef(-1.5f, 0.0f, -6.0f);           // Move left and into the screen
    // glRotatef(anglePyramid, 1.0f, 1.0f, 0.0f);  // Rotate about the (1,1,0)-axis

    // glBegin(GL_TRIANGLES);  // Begin drawing the pyramid with 4 triangles
    // // Front
    // glColor3f(1.0f, 0.0f, 0.0f);  // Red
    // glVertex3f(0.0f, 1.0f, 0.0f);
    // glColor3f(0.0f, 1.0f, 0.0f);  // Green
    // glVertex3f(-1.0f, -1.0f, 1.0f);
    // glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    // glVertex3f(1.0f, -1.0f, 1.0f);

    // // Right
    // glColor3f(1.0f, 0.0f, 0.0f);  // Red
    // glVertex3f(0.0f, 1.0f, 0.0f);
    // glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    // glVertex3f(1.0f, -1.0f, 1.0f);
    // glColor3f(0.0f, 1.0f, 0.0f);  // Green
    // glVertex3f(1.0f, -1.0f, -1.0f);

    // // Back
    // glColor3f(1.0f, 0.0f, 0.0f);  // Red
    // glVertex3f(0.0f, 1.0f, 0.0f);
    // glColor3f(0.0f, 1.0f, 0.0f);  // Green
    // glVertex3f(1.0f, -1.0f, -1.0f);
    // glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    // glVertex3f(-1.0f, -1.0f, -1.0f);

    // // Left
    // glColor3f(1.0f, 0.0f, 0.0f);  // Red
    // glVertex3f(0.0f, 1.0f, 0.0f);
    // glColor3f(0.0f, 0.0f, 1.0f);  // Blue
    // glVertex3f(-1.0f, -1.0f, -1.0f);
    // glColor3f(0.0f, 1.0f, 0.0f);  // Green
    // glVertex3f(-1.0f, -1.0f, 1.0f);
    // glEnd();  // Done drawing the pyramid
    angleCube -= 0.5f;

    glutSwapBuffers();  // Swap the front and back frame buffers (double buffering)
}

void timer(int value) {
    glutPostRedisplay();  // Post re-paint request to activate display()
    if (keys['\033'])
        glutDestroyWindow(glutGetWindow());
    glutTimerFunc(refreshMills, timer, 0);  // next timer call milliseconds later
}

/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
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
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
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