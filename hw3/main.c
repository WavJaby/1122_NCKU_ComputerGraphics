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
#include "gl_text.h"
#include "gl_user_input.h"
#include "gl_vector.h"
#include "stl_reader.h"
#include "math3d.h"
#ifdef _WIN32

#endif

#define FPS 120

char title[32] = "F74114760 hw3";
const int tickMills = 1000 / 60;
int refreshMills = 1000 / FPS;  // refresh interval in milliseconds

int windowWidth, windowHeight;
float windowAspect;
char fpsInfo[40], windowInfo[32];
bool debugText = true, welcome = false;
GLuint xzGridList, depthMap, depthMapFBO;
GLfloat factor = 1.0f;  // for polygon offset

GameObject* gameObjects[10];
GLfloat lightPosition[] = {100.0, 200.0, -100.0};
GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat diffuseLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
GLint shadowWidth = 4096, shadowHeight = 4096;
M3DMatrix44f textureMatrix;

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

void renderGameObjects(bool updateGlobalValue) {
    glColor3f(1, 1, 1);
    for (size_t i = 0; i < 10; i++) {
        if (!gameObjects[i]) break;
        renderGameObject(gameObjects[i], updateGlobalValue ? (Matrix44f)identity : NULL);
    }
}

void updateShadowMap() {
    GLfloat lightToSceneDistance, nearPlane;
    GLfloat lightModelview[16], lightProjection[16];
    GLfloat sceneBoundingRadius = 10.0f;  // based on objects in scene

    // Save the depth precision for where it's useful
    lightToSceneDistance = sqrt(lightPosition[0] * lightPosition[0] +
                                lightPosition[1] * lightPosition[1] +
                                lightPosition[2] * lightPosition[2]);
    nearPlane = lightToSceneDistance - sceneBoundingRadius;

    // Keep the scene filling the depth texture
    // GLfloat fieldOfView = (GLfloat)m3dRadToDeg(2.0f * atan(sceneBoundingRadius / lightToSceneDistance));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-sceneBoundingRadius, sceneBoundingRadius, -sceneBoundingRadius, sceneBoundingRadius, nearPlane, nearPlane + (2.0f * sceneBoundingRadius));
    // gluPerspective(fieldOfView, 1.0f, nearPlane, nearPlane + (2.0f * sceneBoundingRadius));
    glGetFloatv(GL_PROJECTION_MATRIX, lightProjection);
    // Switch to light's point of view
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(lightPosition[0], lightPosition[1], lightPosition[2], 0, 0, 0, 0, 1, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightModelview);

    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);  // Clear the depth buffer

    // Overcome imprecision
    glEnable(GL_POLYGON_OFFSET_FILL);
    renderGameObjects(false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Restore normal drawing state
    glDisable(GL_POLYGON_OFFSET_FILL);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Set up texture matrix for shadow map projection
    M3DMatrix44f tempMatrix;
    m3dLoadIdentity44(tempMatrix);
    m3dTranslateMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
    m3dScaleMatrix44(tempMatrix, 0.5f, 0.5f, 0.5f);
    m3dMatrixMultiply44(textureMatrix, tempMatrix, lightProjection);
    m3dMatrixMultiply44(tempMatrix, textureMatrix, lightModelview);
    // transpose to get the s, t, r, and q rows for plane equations
    m3dTransposeMatrix44(textureMatrix, tempMatrix);
}

void debugShadowMap() {
    // Display shadow map for educational purposes
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(300.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(300.0f, 300.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 300.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

/* Initialize OpenGL Graphics */
void initGL() {
    glewInit();
    printf("GL version: %s\n", glGetString(GL_VERSION));
    int maxViewPortSize;
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &maxViewPortSize);
    printf("Max viewport size: %d\n", maxViewPortSize);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Set background color to black and opaque
    glClearDepth(1.0f);                    // Set background depth to farthest
    glEnable(GL_DEPTH_TEST);               // Enable depth testing for z-culling
    glDepthFunc(GL_LEQUAL);                // Set the type of depth-test
    glPolygonOffset(factor, 0.0f);

    glShadeModel(GL_SMOOTH);                            // Enable smooth shading
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    glEnable(GL_CULL_FACE);  // Do not calculate inside of jet
    glFrontFace(GL_CCW);     // Counter clock-wise polygons face out

    // Light
    GLfloat specularLight[] = {1.0, 1.0, 1.0, 1.0};
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    GameObject* block = newGameObjectDefault(loadStlModel("../model/block.stl"));
    GameObject* cog = newGameObjectDefault(loadStlModel("../model/cog.stl"));
    GameObject* base = newGameObjectPosRot((Vector3f){0, 0, 0}, (Vector3f){0, 90, 0}, loadStlModel("../model/base.stl"));
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

    GLuint listCache = glGenLists(3);
    glNewList(listCache, GL_COMPILE);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    drawRect(-10, -10, 20, 20);
    glPopMatrix();
    glEndList();
    gameObjects[0] = newGameObjectDefault(listCache++);

    gameObjects[1] = block;

    // sphere
    glNewList(listCache, GL_COMPILE);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(0.25f, 20, 20);
    glEndList();
    gameObjects[2] = newGameObjectDefault(listCache++);
    vec3fSet(gameObjects[2]->position, (Vector3f){-1.5f, 0.25f, 0});

    // cube
    // glNewList(listCache, GL_COMPILE);
    // glColor3f(1.0f, 1.0f, 0.0f);
    // glutSolidCube(0.5f);
    // glEndList();
    // gameObjects[2] = newGameObjectDefault(listCache++);
    // vec3fSet(gameObjects[2]->position, (Vector3f){1.5f, 0.25f, 0});
    // gameObjects[2]->collider = (Collider){COLIDER_BOX, {0, 0, 0}, {1, 1, 1}};
    // GLfloat lightModelview[16];
    // glGetFloatv(GL_MODELVIEW_MATRIX, lightModelview);

    // // Draw magenta torus
    // glColor3f(1.0f, 0.0f, 1.0f);
    // glPushMatrix();
    // glTranslatef(2.0f, 1.0f, -1.0f);
    // glutSolidTorus(0.5f, 1.0f, 30, 50);
    // glPopMatrix();

    // XZ grid plane
    xzGridList = debugGridCreate(10);

    // Text texture init
    glTextInit();

    vec3fSet(cameraPos, (Vector3f){0, 2, -5});
    vec3fSet(cameraAngle, (Vector3f){-10, 90, 0});

    // Shadow
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    // if (ambientShadowAvailable)
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.5f);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display() {
    updateShadowMap();

    glViewport(0, 0, windowWidth, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    calculateCameraMatrix();
    glPushMatrix();

    // Setup enviroment
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear color and depth buffers

    // if (!ambientShadowAvailable) {
    GLfloat lowAmbient[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat lowDiffuse[4] = {0.35f, 0.35f, 0.35f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lowAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lowDiffuse);
    renderGameObjects(false);
    // Enable alpha test so that shadowed fragments are discarded
    glAlphaFunc(GL_GREATER, 0.9f);
    glEnable(GL_ALPHA_TEST);
    // }

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    // Set up shadow comparison
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

    // Set up the eye plane for projecting the shadow map on the scene
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);
    glTexGenfv(GL_S, GL_EYE_PLANE, &textureMatrix[0]);
    glTexGenfv(GL_T, GL_EYE_PLANE, &textureMatrix[4]);
    glTexGenfv(GL_R, GL_EYE_PLANE, &textureMatrix[8]);
    glTexGenfv(GL_Q, GL_EYE_PLANE, &textureMatrix[12]);
    // Render GameObjects
    renderGameObjects(true);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);
    glPopMatrix();
    glDisable(GL_LIGHTING);

    // Render xz grid
    debugGridRender(xzGridList);
    // GameObject* base = listT_get(&gameObjects[1]->childs, GameObject*, 1);
    // GameObject* lowerBody = listT_get(&base->childs, GameObject*, 0);
    // GameObject* upperBody = listT_get(&lowerBody->childs, GameObject*, 0);
    // GameObject* clawBase = listT_get(&upperBody->childs, GameObject*, 0);
    // glPushMatrix();
    // glTranslatef(vx(clawBase->globalPosition), vy(clawBase->globalPosition), vz(clawBase->globalPosition));
    // glRotatef(vx(clawBase->globalRotation) * Rad2Deg, 1, 0, 0);
    // glRotatef(vy(clawBase->globalRotation) * Rad2Deg, 0, 1, 0);
    // glRotatef(vz(clawBase->globalRotation) * Rad2Deg, 0, 0, 1);
    // glTranslatef(0, 0, 0.5);
    // printf(vec3PrintFmt("%.2f") "\n", vec3Print(clawBase->globalRotation));
    // glBegin(GL_LINES);
    // glVertex3f(0, 0, 0);
    // glVertex3f(0, 0, 1);
    // glEnd();
    // glPopMatrix();

    // vec3fSet(gameObjects[2]->position, clawBase->globalPosition);

    // Init 2D UI projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, windowWidth, 0, windowHeight, 0.0, 1.0);
    // Render 2D UI component
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (debugText) {
        glColor4f(1, 1, 1, 1);
        int textY = windowHeight;
        glDrawString(fpsInfo, strlen(fpsInfo), 5, textY -= 20, 16);
        glDrawString(windowInfo, strlen(windowInfo), 5, textY -= 20, 16);
        char strCache[128];
        sprintf(strCache, "Camera pos: (%.2f, %.2f, %.2f), angle: (%.2f, %.2f)",
                vz(cameraPos), vy(cameraPos), vz(cameraPos),
                vx(cameraAngle), vy(cameraAngle));
        glDrawString(strCache, strlen(strCache), 5, textY -= 20, 16);
        sprintf(strCache, "Velocity: (%.2f, %.2f, %.2f)",
                vx(cameraVelocity), vy(cameraVelocity), vz(cameraVelocity));
        glDrawString(strCache, strlen(strCache), 5, textY -= 20, 16);
        glDrawString("Press F3 to toggle debug text", 29, 5, textY -= 20, 16);
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
    // debugShadowMap();
    // Reset projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Update screen
    frameUpdate();
    glutSwapBuffers();
}

bool grab = false;
void updateGame() {
    GameObject* cog = listT_get(&gameObjects[1]->childs, GameObject*, 0);
    vy(cog->rotation) += 180 * deltaTimeTick;
    if (vy(cog->rotation) > 360) vy(cog->rotation) -= 360;
    if (vy(cog->rotation) < 0) vy(cog->rotation) += 360;

    GameObject* base = listT_get(&gameObjects[1]->childs, GameObject*, 1);
    GameObject* lowerBody = listT_get(&base->childs, GameObject*, 0);
    GameObject* upperBody = listT_get(&lowerBody->childs, GameObject*, 0);
    GameObject* clawBase = listT_get(&upperBody->childs, GameObject*, 0);
    if (keys['Q']) vy(base->rotation) += 180 * deltaTimeTick;
    if (keys['A']) vy(base->rotation) -= 180 * deltaTimeTick;

    if (keys['W']) vx(lowerBody->rotation) += 180 * deltaTimeTick;
    if (keys['S']) vx(lowerBody->rotation) -= 180 * deltaTimeTick;
    if (vx(lowerBody->rotation) > 0) vx(lowerBody->rotation) = 0;
    if (vx(lowerBody->rotation) < -180) vx(lowerBody->rotation) = -180;
    float angle = (vx(lowerBody->rotation) + 90) * 1.4;

    if (keys['E']) vx(upperBody->rotation) += 180 * deltaTimeTick;
    if (keys['D']) vx(upperBody->rotation) -= 180 * deltaTimeTick;
    if (angle > 0) {
        if (vx(upperBody->rotation) > 150 - angle) vx(upperBody->rotation) = 150 - angle;
        if (vx(upperBody->rotation) < -150) vx(upperBody->rotation) = -150;
    } else {
        if (vx(upperBody->rotation) > 150) vx(upperBody->rotation) = 150;
        if (vx(upperBody->rotation) < -150 - angle) vx(upperBody->rotation) = -150 - angle;
    }

    if (keys['R']) vx(clawBase->rotation) += 180 * deltaTimeTick;
    if (keys['F']) vx(clawBase->rotation) -= 180 * deltaTimeTick;
    if (vx(clawBase->rotation) > 100) vx(clawBase->rotation) = 100;
    if (vx(clawBase->rotation) < -100) vx(clawBase->rotation) = -100;

    Matrix44f m = identity, cache = identity;
    mat44fTranslate(cache, clawBase->globalPosition);
    mat44fMultiply(m, cache, m);
    mat44fRotationX(cache, vx(clawBase->globalRotation));
    mat44fMultiply(m, cache, m);
    mat44fRotationY(cache, vy(clawBase->globalRotation));
    mat44fMultiply(m, cache, m);
    mat44fRotationZ(cache, vz(clawBase->globalRotation));
    mat44fMultiply(m, cache, m);
    mat44fTranslate(cache, (Vector3f){0, 0, 0.5});
    mat44fMultiply(m, cache, m);
    Vector3f pos, rot;
    mat44fGetPosition(m, pos);
    // Grab
    GameObject* ball = gameObjects[2];
    if (keys['T']) {
        if (vec3fDistance(ball->position, pos) < 0.25)
            grab = true;
    }
    // Release
    if (keys['G'] && grab) {
        grab = false;
    }

    if (grab)
        vec3fSet(ball->position, pos);

    // printf(vec3PrintFmt("%.4f") "\n", vec3Print(clawBase->globalPosition));
}

void fpsUpdate(float fps, float tick) {
    sprintf(fpsInfo, "fps:%7.2f, tick: %.2f, d: %.5f", fps, tick, deltaTimeTick);
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
    sprintf(windowInfo, "Window: %dx%d\n", width, height);

    updateShadowMap();
    firstPersonWindowSizeUpdate(width, height);

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
    if (glutStoped) return;
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