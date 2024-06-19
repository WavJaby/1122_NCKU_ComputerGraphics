#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define WJCL_HASH_MAP_IMPLEMENTATION
#define WJCL_LINKED_LIST_IMPLEMENTATION
#include <WJCL/map/wjcl_hash_map.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

// Static lib
#include "linmath.h"
#include "glfw_camera.h"
#include "fps_counter.h"
#include "gl_shader.h"
#include "gl_text.h"
#include "gl_texture.h"
#include "gl_mesh.h"
// Project lib
#include "depth_shader.h"
#include "object_shader.h"
#include "game/chunk/chunk_sub.h"
#include "game/chunk/chunk.h"
#include "game/block/block.h"

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[] = "Final Project";

// Game
#include "game_manager.h"

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

static void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void onWindowSizeChange(GLFWwindow* window, int width, int height) {
    printf("%dx%d\n", width, height);
    windowWidth = width;
    windowHeight = height;
    camera_windowSizeUpdate(width, height);
    ui_windowSizeUpdate(width, height);
    glViewport(0, 0, width, height);
}

void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

double lastMouseX = -1, lastMouseY = -1, mouseSensitivity = 6;
void onCursorMove(GLFWwindow* window, double xpos, double ypos) {
    if (lastMouseX == -1) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        return;
    }

    cameraAngle[0] += (ypos - lastMouseY) * mouseSensitivity * deltaTime;
    cameraAngle[1] += (xpos - lastMouseX) * mouseSensitivity * deltaTime;
    if (cameraAngle[0] > 90)
        cameraAngle[0] = 90;
    else if (cameraAngle[0] < -90)
        cameraAngle[0] = -90;

    // printf("%f, %f\n", cameraAngle[0], cameraAngle[1]);

    lastMouseX = xpos;
    lastMouseY = ypos;
}

char fpsInfo[64];
void fpsUpdate(float fps, float tick) {
    sprintf(fpsInfo, "fps:%7.2f, tick: %.2f, d: %.5f", fps, tick, deltaTime);
}

vec3 lightDir = {0, -1.0, 0.5};
float lightAngle = 0;
mat4x4 lightSpaceMatrix;
GLuint depthMap;

void onStart(GLFWwindow* window) {
    objectShaderInit();
    depthShaderInit();

    // Windows dont fire FramebufferSizeCallback at start
    camera_windowSizeUpdate(windowWidth, windowHeight);
    ui_windowSizeUpdate(windowWidth, windowHeight);

    // Init input event
    glfwSetFramebufferSizeCallback(window, onWindowSizeChange);
    glfwSetKeyCallback(window, onKeyPress);

    gameManagerOnStart();

    cameraPos[0] = 0;
    cameraPos[1] = 100;
    cameraPos[2] = 0;

    cameraAngle[0] = 45;

    // Calculate light
    vec3_norm(lightDir, lightDir);
    lightDir[0] = cos(lightAngle * M_DEG_2_RAD);
}

void onRenderShadow(GLFWwindow* window) {
    float near_plane = 0.f, far_plane = 1000.f;
    mat4x4 lightProjection;
    mat4x4_ortho(lightProjection, -50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);

    vec3 origin = {cameraPos[0], cameraPos[1], cameraPos[2]};
    vec3 lightPos;
    vec3_scale(lightPos, lightDir, -1);
    vec3_add(lightPos, lightPos, origin);
    mat4x4 lightView;
    mat4x4_look_at(lightView, lightPos, origin, (vec3){0, 1, 0});

    mat4x4_mul(lightSpaceMatrix, lightProjection, lightView);

    glUseProgram(depthShader.program);
    glUniformMatrix4fv(depthShader.uLlightSpaceMatrix, 1, GL_FALSE, (float*)lightSpaceMatrix);

    gameManagerOnRenderShadow();
}

void onRender(GLFWwindow* window) {
    static int moveSpeed = 50;

    vec3 dir = {cameraDir[0], 0, cameraDir[2]}, v = {0, 0, 0};
    // vec3_norm(dir, dir);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        vec3_add(v, v, dir);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        vec3_sub(v, v, dir);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 right;
        vec3_mul_cross(right, dir, vec3_up);
        vec3_sub(v, v, right);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 right;
        vec3_mul_cross(right, dir, vec3_up);
        vec3_add(v, v, right);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        v[1] = 1;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        v[1] = -1;

    vec3_scale(v, v, deltaTime * moveSpeed);
    vec3_add(cameraPos, cameraPos, v);

    // Render model
    glUseProgram(objectShader.program);
    glUniformMatrix4fv(objectShader.uProjection, 1, GL_FALSE, (float*)cameraProjectionMat);
    glUniformMatrix4fv(objectShader.uView, 1, GL_FALSE, (float*)cameraViewMat);
    glUniformMatrix4fv(objectShader.uLightSpaceMatrix, 1, GL_FALSE, (float*)lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(objectShader.uShadowMap, 1);
    glUniform3fv(objectShader.uViewPos, 1, cameraPos);

    glUniform3fv(objectShader.dirLight_direction, 1, (const GLfloat*)lightDir);
    glUniform4f(objectShader.dirLight_color, 1, 1, 1, 1);
    glUniform1f(objectShader.dirLight_ambient, 0.1);
    glUniform1f(objectShader.dirLight_diffuse, 0.8);

    glUniform1f(objectShader.material_shininess, 0);

    glUniform1i(objectShader.material_useDiffuse, 1);

    gameManagerOnRender();

    // Render UI
    glDisable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    gameManagerOnRenderUI();

    // Debug UI
    ui_textDrawString(fpsInfo, 5, 20, 20);
    char infoCache[128];
    sprintf(infoCache, "Camera pos: (%.2f, %.2f, %.2f), dir: (%.2f, %.2f, %.2f)",
            cameraPos[0], cameraPos[1], cameraPos[2],
            cameraDir[0], cameraDir[1], cameraDir[2]);
    ui_textDrawString(infoCache, 5, 20 * 2, 20);

    ui_drawTexture(depthMap, 0, 50, 200, 200, 0);

    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char* argv[]) {
    printf("Program start\n");

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // Set minimum version required
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Create window
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Init context setting
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        printf("Raw mouse motion support\n");
    }
    glfwSetCursorPosCallback(window, onCursorMove);
    glfwSwapInterval(1);

    // Init gl settings
    int version = gladLoadGL(glfwGetProcAddress);
    printf("GL Version %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Init depthMap
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Init frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Init UI render
    ui_init();

    fpsCounterInit();
    uint64_t frame = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        camera_updateViewMatrix();

        // Init
        if (frame == 1)
            onStart(window);
        else if (frame != 0) {
            glEnable(GL_DEPTH_TEST);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            // Render shader scene
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            onRenderShadow(window);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Render scene
            glViewport(0, 0, windowWidth, windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            onRender(window);
        }

        glfwSwapBuffers(window);
        frameUpdate(fpsUpdate);
        frame++;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}