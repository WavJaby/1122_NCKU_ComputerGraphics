#include <GL/glew.h>
#include <GL/glut.h>
#include <stdbool.h>
#include <stdio.h>

#define MEM_TRACK
#define WJCL_HASH_MAP_IMPLEMENTATION
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_LIST_TYPE_IMPLEMENTATION
#include "WJCL/memory/wjcl_mem_track.h"
#include "debug_grid.h"
#include "fps_counter.h"
#include "game_object.h"
#include "gl_first_person_control.h"
#include "gl_user_input.h"
#include "gl_vector.h"
#ifdef _WIN32

#endif

#define FPS 120

char title[32] = "F74114760 hw2";
const int tickMills = 1000 / 60;
int refreshMills = 1000 / FPS;  // refresh interval in milliseconds

int windowWidth, windowHeight;
float windowAspect;
GLuint xzGridList;
GameObject* gameObjects[10];
GLfloat lightPosition[] = {2.0, 10.0, -2.0, 1.0};
char fpsInfo[64];

void fpsUpdate(float fps, float tick) {
    sprintf(fpsInfo, "fps: %.2f, tick: %.2f, delta: %.6f", fps, tick, deltaTimeUpdate);
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(x, y + h, 0.0f);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(x, y, 0.0f);
    glNormal3f(0, 0, 1);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x + w, y, 0.0f);
    glNormal3f(0, 0, 1);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x + w, y + h, 0.0f);
    glEnd();
}

/* Initialize OpenGL Graphics */
void initGL() {
    glewInit();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);               // Set background color to black and opaque
    glClearDepth(1.0f);                                 // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);                            // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);                             // Set the type of depth-test
    glShadeModel(GL_SMOOTH);                            // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_NORMALIZE);
    glFrontFace(GL_CCW);     // Counter clock-wise polygons face out
    glEnable(GL_CULL_FACE);  // Do not calculate inside of jet

    // Light
    GLfloat ambientLight[] = {0.001, 0.001, 0.001, 1.0};
    GLfloat diffuseLight[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat specularLight[] = {1.0, 1.0, 1.0, 1.0};
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // XZ grid plane
    xzGridList = debugGridCreate(10);

    cameraPos = (Vector3f){0, 2, -3};
    cameraAngle = (Vector3f){0, 90, 0};
    
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    printf("Init done\n");
}

void renderGameObjects() {
    glColor3f(1, 1, 1);
    for (size_t i = 0; i < 10; i++) {
        if (!gameObjects[i]) break;
        renderGameObject(gameObjects[i]);
    }

    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawRect(-10, -10, 20, 20);
    glPopMatrix();

    // Draw red cube
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(1.0f, 1.0f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw magenta torus
    glColor3f(1.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(2.0f, 1.0f, -1.0f);
    glutSolidTorus(0.5f, 1.0f, 20, 50);
    glPopMatrix();
}

void display() {
    glMatrixMode(GL_MODELVIEW);
    calculateCameraMatrix();
    glViewport(0, 0, windowWidth, windowHeight);
    glPushMatrix();

    // Setup enviroment
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers

    // glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    // glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    // Render GameObjects
    renderGameObjects();
    glPopMatrix();
    glDisable(GL_LIGHTING);

    // Render xz grid
    debugGridRender(xzGridList);

    // Init 2D UI projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, 0.0, 1.0);
    // Render 2D UI component
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Reset projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Update screen
    frameUpdate();
    glutSwapBuffers();
}

void updateGame() {
}

void update() {
    userInputInitUpdate();
    calculateCameraMovement();

    updateGame();

    tickUpdate(fpsUpdate);
}

void reshape(GLsizei width, GLsizei height) {
    // Compute aspect ratio of the new window
    if (height == 0)
        height = 1;  // To prevent divide by 0
    windowWidth = width;
    windowHeight = height;
    firstPersonWindowSizeUpdate(width, height);

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping volume to match the viewport
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    windowAspect = (float)width / (float)height;
    // Enable perspective projection with fovy, aspect, zNear and zFar
    gluPerspective(45.0f, windowAspect, 0.1f, 1000.0f);
}

bool glutStoped = false;
void frameTimer(int value) {
    if (keys[GLUT_KEY_ESC]) {
        glutStoped = true;
        glutDestroyWindow(glutGetWindow());
        memTrackResult();
        return;
    }
    glutPostRedisplay();  // Post re-paint request to activate display()
    glutTimerFunc(refreshMills, frameTimer, 0);
}

void updateTimer(int value) {
    if (glutStoped)
        return;
    update();
    glutTimerFunc(tickMills, updateTimer, 0);
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);  // Enable double buffered mode
    glutInitWindowSize(1280, 720);                              // Set the window width & height
    glutInitWindowPosition(0, 0);                               // Position the window
    glutCreateWindow(title);                                    // Create window with title
    glutDisplayFunc(display);                                   // Register callback handler for window re-paint event
    glutReshapeFunc(reshape);                                   // Register callback handler for window re-size event
    userInputInit();
    firstPersonInit();
    initGL();
    fpsCounterInit();
    glutTimerFunc(0, frameTimer, 0);
    glutTimerFunc(0, updateTimer, 0);
    glutMainLoop();
    return 0;
}