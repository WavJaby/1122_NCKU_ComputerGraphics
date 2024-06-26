#include <GL/glut.h>
#include <stdio.h>
#include <stdbool.h>

#include "lib/fps_counter.h"
#include "lib/gl_first_person_control.h"
#include "lib/gl_user_input.h"
#include "lib/gl_vector.h"
#include "lib/gl_text_old.h"
#include "lib/stl_reader.h"
#include "lib/debug_grid.h"
#ifdef _WIN32
#include "open_file_dialog.h"
#endif

#define FPS 120

char title[32] = "F74114760 hw1";
const int tickMills = 1000 / 60;
int refreshMills = 1000 / FPS;  // refresh interval in milliseconds

int windowWidth, windowHeight;
float windowAspect;
STrianglesInfo triInfo;
GLuint xzGridList;
GLuint displayList;
GLfloat angle = 0;
GLfloat lightPosition[] = {1.0, 10.0, -1.0, 1.0};
char fpsInfo[64];
bool debugText = true, welcome = true;

void fpsUpdate(float fps, float tick) {
    sprintf(fpsInfo, "fps: %.2f, tick: %.2f, delta: %.6f", fps, tick, deltaTimeUpdate);
    // printf("%s\n", cBuffer);
    // glutSetWindowTitle(cBuffer);
}

void loadModel(char* path) {
    free(triInfo.triangles);
    loadStl(path, &triInfo);
    printf("Triangles: %d\nAuto Scale: %f\n", triInfo.trianglesCount, 1 / triInfo.maxSize);

    // Create model
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < triInfo.trianglesCount; ++i) {
        // float* normalRaw = triInfo.triangles[i].normal;
        // Vector3f normal = {.x = normalRaw[0], .y = normalRaw[1], .z = normalRaw[2]};
        // printf("%f\n", vec3fLength(normal));
        // glNormal3f(normal.x, normal.y, normal.z);
        glNormal3fv(triInfo.triangles[i].normal);
        glVertex3f(
            triInfo.triangles[i].a[0] - vx(triInfo.center),
            triInfo.triangles[i].a[1] - vy(triInfo.center),
            triInfo.triangles[i].a[2] - vz(triInfo.center));
        glVertex3f(
            triInfo.triangles[i].b[0] - vx(triInfo.center),
            triInfo.triangles[i].b[1] - vy(triInfo.center),
            triInfo.triangles[i].b[2] - vz(triInfo.center));
        glVertex3f(
            triInfo.triangles[i].c[0] - vx(triInfo.center),
            triInfo.triangles[i].c[1] - vy(triInfo.center),
            triInfo.triangles[i].c[2] - vz(triInfo.center));
    }
    glEnd();
    glEndList();
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(x, y + h, 0.0f);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(x, y, 0.0f);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(x + w, y, 0.0f);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(x + w, y + h, 0.0f);
    glEnd();
}

/* Initialize OpenGL Graphics */
void initGL() {
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

    // Light
    GLfloat ambientLight[] = {0.001, 0.001, 0.001, 1.0};
    GLfloat diffuseLight[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat specularLight[] = {1.0, 1.0, 1.0, 1.0};
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    loadModel("Bunny_Binary.stl");

    // XZ grid plane
    xzGridList = debugGridCreate(10);

    // Text texture init
    glTextInit();

    vec3fSet(cameraPos, (Vector3f){0, 1.5, -3});
    vec3fSet(cameraAngle, (Vector3f){0, 90, 0});
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);
    calculateCameraMatrix();

    // Lighting
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    // Render model
    glPushMatrix();
    glEnable(GL_LIGHTING);
    glColor3f(1, 1, 1);
    glTranslatef(0, 1.5, 0);
    glRotatef(-90, 1, 0, 0);
    glRotatef(angle, 0, 1, 0);
    float scale = 1 / triInfo.maxSize;
    glScalef(scale, scale, scale);
    glCallList(displayList);
    glPopMatrix();
    // Render xz grid
    debugGridRender(xzGridList);

    // Render UI Begin
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // printf("%f\n", windowAspect);

    glDisable(GL_LIGHTING);
    if (debugText) {
        glColor4f(1, 1, 1, 1);
        glDrawString(fpsInfo, strlen(fpsInfo), 5, windowHeight - 20, 16);
        char cameraInfo[128];
        sprintf(cameraInfo, "Camera pos: (%.2f, %.2f, %.2f), angle: (%.2f, %.2f)",
                vx(cameraPos), vy(cameraPos), vz(cameraPos),
                vx(cameraAngle), vy(cameraAngle));
        glDrawString(cameraInfo, strlen(cameraInfo), 5, windowHeight - 40, 16);
        sprintf(cameraInfo, "Velocity: (%.2f, %.2f, %.2f)",
                vx(cameraVelocity), vy(cameraVelocity), vz(cameraVelocity));
        glDrawString(cameraInfo, strlen(cameraInfo), 5, windowHeight - 60, 16);
        glDrawString("Press F3 to toggle debug text", 29, 5, windowHeight - 80, 16);
    }
    if (welcome) {
        char l1[] = "For more control, see README";
        char l2[] = "Press enter to continue";
        int size = 64;
        glColor4f(0, 0, 0, 0.5);
        drawRect(0, 0, windowWidth, windowHeight);
        glColor4f(1, 1, 1, 1);
        glDrawStringCenter(l1, sizeof(l1), windowWidth / 2, windowHeight / 2 + 32, size);
        glDrawStringCenter(l2, sizeof(l2), windowWidth / 2, windowHeight / 2 - 32, size);
    }

    // Render UI End
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Update screen
    frameUpdate();
    glutSwapBuffers();
}

bool openFileDone = false;
Vector3f cameraAngleSave;
void update() {
    userInputInitUpdate();
    calculateCameraMovement();

    if (keysOnPress[GLUT_KEY_ENTER])
        welcome = false;
    if (welcome)
        return;

    if (keys['X'])
        angle += 90 * deltaTimeUpdate;
    if (keys['Z'])
        angle -= 90 * deltaTimeUpdate;
    if (keysOnPress[GLUT_KEY_F3])
        debugText = !debugText;

// Open model
#ifdef _WIN32
    if (keysOnPress['O']) {
        vec3fSet(cameraAngleSave, cameraAngle);
        char fileFullPath[260] = {0};
        if (openFileDialog(fileFullPath, sizeof(fileFullPath))) {
            loadModel(fileFullPath);
        }
        openFileDone = true;
        return;
    }
    if (openFileDone) {
        openFileDone = false;
        firstPersonMouseReset();
        vec3fSet(cameraAngle, cameraAngleSave);
        return;
    }
#endif

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