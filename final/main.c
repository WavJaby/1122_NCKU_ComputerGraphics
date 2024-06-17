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
#include "object_shader.h"
#include "game/chunk/chunk_sub.h"
#include "game/chunk/chunk.h"
#include "game/block/block.h"

int windowWidth = 640;
int windowHeight = 480;
char windowTitle[] = "Final Project";

// Game
#include "game_manager.h"

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

void onStart(GLFWwindow* window) {
    objectShaderInit();

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
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glUseProgram(objectShader.program);
    glUniformMatrix4fv(objectShader.uProjection, 1, GL_FALSE, (float*)cameraProjectionMat);
    glUniformMatrix4fv(objectShader.uView, 1, GL_FALSE, (float*)cameraViewMat);
    glUniform3fv(objectShader.viewPos, 1, cameraPos);

    vec3 lightDir = {0.5, -1, 0.5};
    vec3_norm(lightDir, lightDir);
    glUniform3fv(objectShader.dirLight_direction, 1, (const GLfloat*)lightDir);
    glUniform4f(objectShader.dirLight_color, 1, 1, 1, 1);
    glUniform1f(objectShader.dirLight_ambient, 0.1);
    glUniform1f(objectShader.dirLight_diffuse, 0.8);

    glUniform1f(objectShader.material_shininess, 0);

    glActiveTexture(GL_TEXTURE0);
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

    // Init UI render
    ui_init();

    fpsCounterInit();
    uint64_t frame = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_updateViewMatrix();

        // Init
        if (frame == 1)
            onStart(window);
        else if (frame != 0)
            onRender(window);

        glfwSwapBuffers(window);
        frameUpdate(fpsUpdate);
        frame++;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}