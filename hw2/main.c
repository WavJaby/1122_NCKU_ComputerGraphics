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
#include "gl_text.h"
#include "gl_user_input.h"
#include "gl_vector.h"
#include "stl_reader.h"
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
bool debugText = true, welcome = true;

void fpsUpdate(float fps, float tick) {
    sprintf(fpsInfo, "fps: %.2f, tick: %.2f, delta: %.6f", fps, tick, deltaTimeUpdate);
}

GLuint loadStlModel(char* path) {
    STrianglesInfo triInfo;
    loadStl(path, &triInfo);
    printf("Triangles: %d\n", triInfo.trianglesCount);

    // Create model
    GLuint displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < triInfo.trianglesCount; ++i) {
        glNormal3fv(triInfo.triangles[i].normal);
        glVertex3f(
            triInfo.triangles[i].a[0],
            triInfo.triangles[i].a[1],
            triInfo.triangles[i].a[2]);
        glVertex3f(
            triInfo.triangles[i].b[0],
            triInfo.triangles[i].b[1],
            triInfo.triangles[i].b[2]);
        glVertex3f(
            triInfo.triangles[i].c[0],
            triInfo.triangles[i].c[1],
            triInfo.triangles[i].c[2]);
    }
    glEnd();
    glEndList();
    free(triInfo.triangles);
    return displayList;
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

    GameObject* block = newGameObjectDefault(loadStlModel("../model/block.stl"));
    GameObject* cog = newGameObjectDefault(loadStlModel("../model/cog.stl"));
    GameObject* base = newGameObjectPosRot((Vector3f){0, 0, 0}, (Vector3f){0, 70, 0}, loadStlModel("../model/base.stl"));
    GameObject* lowerBody = newGameObjectPosRot((Vector3f){0, 14 / 16.f, 0}, (Vector3f){-90, 0, 0}, loadStlModel("../model/lower_body.stl"));
    GameObject* upperBody = newGameObjectPosRot((Vector3f){0, 0, 1}, (Vector3f){90, 0, 0}, loadStlModel("../model/upper_body.stl"));
    GameObject* clawBase = newGameObjectPosRot((Vector3f){0, 0, 15 / 16.f}, (Vector3f){0, 0, 0}, loadStlModel("../model/claw_base.stl"));
    GameObject* lowerGrip = newGameObjectPosRot((Vector3f){0, -1.5 / 16.f, 0}, (Vector3f){0, 0, 0}, loadStlModel("../model/lower_claw_grip.stl"));
    GameObject* upperGrip = newGameObjectPosRot((Vector3f){0, 1.5 / 16.f, 0}, (Vector3f){0, 0, 0}, loadStlModel("../model/upper_claw_grip.stl"));

    gameobjectAddChild(block, cog);
    gameobjectAddChild(block, base);
    gameobjectAddChild(base, lowerBody);
    gameobjectAddChild(lowerBody, upperBody);
    gameobjectAddChild(upperBody, clawBase);
    gameobjectAddChild(clawBase, lowerGrip);
    gameobjectAddChild(clawBase, upperGrip);
    gameObjects[0] = block;

    // XZ grid plane
    xzGridList = debugGridCreate(10);

    // Text texture init
    glTextInit();

    vec3fSet(cameraPos, (Vector3f){0, 2, -5});
    vec3fSet(cameraAngle, (Vector3f){0, 90, 0});
    printf("Init done\n");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers
    glMatrixMode(GL_MODELVIEW);
    calculateCameraMatrix();

    // Lighting
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHTING);
    // Render model
    glPushMatrix();
    glColor3f(1, 1, 1);

    for (size_t i = 0; i < 10; i++) {
        if (!gameObjects[i]) break;
        renderGameObject(gameObjects[i], (Matrix44f)identity);
    }

    glRotatef(-90, 1, 0, 0);
    drawRect(-10, -10, 20, 20);

    glPopMatrix();

    // Render xz grid
    glDisable(GL_LIGHTING);
    debugGridRender(xzGridList);

    // Render UI Begin
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // printf("%f\n", windowAspect);

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

void updateGame() {
    GameObject* cog = listT_get(&gameObjects[0]->childs, GameObject*, 0);
    vy(cog->rotation) += 180 * deltaTimeUpdate;
    if (vy(cog->rotation) > 360) vy(cog->rotation) -= 360;
    if (vy(cog->rotation) < 0) vy(cog->rotation) += 360;

    GameObject* base = listT_get(&gameObjects[0]->childs, GameObject*, 1);
    GameObject* lowerBody = listT_get(&base->childs, GameObject*, 0);
    GameObject* upperBody = listT_get(&lowerBody->childs, GameObject*, 0);
    GameObject* clawBase = listT_get(&upperBody->childs, GameObject*, 0);
    if (keys['Q']) vy(base->rotation) += 180 * deltaTimeUpdate;
    if (keys['A']) vy(base->rotation) -= 180 * deltaTimeUpdate;

    if (keys['W']) vx(lowerBody->rotation) += 180 * deltaTimeUpdate;
    if (keys['S']) vx(lowerBody->rotation) -= 180 * deltaTimeUpdate;
    if (vx(lowerBody->rotation) > 0) vx(lowerBody->rotation) = 0;
    if (vx(lowerBody->rotation) < -180) vx(lowerBody->rotation) = -180;
    float angle = vx(lowerBody->rotation) + 90;

    if (keys['E']) vx(upperBody->rotation) += 180 * deltaTimeUpdate;
    if (keys['D']) vx(upperBody->rotation) -= 180 * deltaTimeUpdate;
    if (angle > 0) {
        if (vx(upperBody->rotation) > 150 - angle) vx(upperBody->rotation) = 150 - angle;
        if (vx(upperBody->rotation) < -150) vx(upperBody->rotation) = -150;
    } else {
        if (vx(upperBody->rotation) > 150) vx(upperBody->rotation) = 150;
        if (vx(upperBody->rotation) < -150 - angle) vx(upperBody->rotation) = -150 - angle;
    }

    if (keys['R']) vx(clawBase->rotation) += 180 * deltaTimeUpdate;
    if (keys['F']) vx(clawBase->rotation) -= 180 * deltaTimeUpdate;
    if (vx(clawBase->rotation) > 100) vx(clawBase->rotation) = 100;
    if (vx(clawBase->rotation) < -100) vx(clawBase->rotation) = -100;
}

void update() {
    userInputInitUpdate();
    calculateCameraMovement();

    if (welcome && keysOnPress[GLUT_KEY_ENTER]) {
        welcome = false;
        firstPersonPause = false;
    }
    if (!welcome) {
        updateGame();
        if (keysOnPress[GLUT_KEY_F3])
            debugText = !debugText;
    }

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
    firstPersonPause = true;
    firstPersonInit();
    initGL();
    fpsCounterInit();
    glutTimerFunc(0, frameTimer, 0);
    glutTimerFunc(0, updateTimer, 0);
    glutMainLoop();
    memTrackResult();
    return 0;
}