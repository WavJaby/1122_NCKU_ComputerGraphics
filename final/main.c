#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "linmath.h"
#include "glfw_camera.h"
#include "fps_counter.h"
#include "mesh.h"
#include "gl_shader.h"
#include "gl_text.h"
#include "game/chunk/chunk_mesh.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

int windowWidth = 640;
int windowHeight = 480;
char windowTitle[] = "Final Project";

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

double lastMouseX = -1, lastMouseY = -1, mouseSensitivity = 10;
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

    glEnable(GL_DEPTH_TEST);

    // Init UI render
    ui_init();

    // Init input event
    glfwSetFramebufferSizeCallback(window, onWindowSizeChange);
    glfwSetKeyCallback(window, onKeyPress);

    // Windows dont fire FramebufferSizeCallback at start
    camera_windowSizeUpdate(windowWidth, windowHeight);
    ui_windowSizeUpdate(windowWidth, windowHeight);

    GLuint program = compileShaderFileProgram("../shaders/object_shader.frag", "../shaders/object_shader.vert");

    GLint uModel = glGetUniformLocation(program, "uModel");
    GLint uView = glGetUniformLocation(program, "uView");
    GLint uProjection = glGetUniformLocation(program, "uProjection");

    GLint viewPos = glGetUniformLocation(program, "viewPos");

    GLint dirLight_direction = glGetUniformLocation(program, "dirLight.direction");
    GLint dirLight_color = glGetUniformLocation(program, "dirLight.base.color");
    GLint dirLight_ambient = glGetUniformLocation(program, "dirLight.base.ambientIntensity");
    GLint dirLight_diffuse = glGetUniformLocation(program, "dirLight.base.diffuseIntensity");

    GLint material_color = glGetUniformLocation(program, "material.color");
    GLint material_useDiffuse = glGetUniformLocation(program, "material.useDiffuse");
    GLint material_diffuse = glGetUniformLocation(program, "material.diffuse");
    GLint material_useSpecular = glGetUniformLocation(program, "material.useSpecular");
    GLint material_specular = glGetUniformLocation(program, "material.specular");
    GLint material_shininess = glGetUniformLocation(program, "material.shininess");

    ChunkSubMesh chunkSubMesh = {};
    chunkSubMesh.vertices_uv_normal = malloc(10000);
    chunkSubMesh.indices = malloc(10000);
    BlockMesh blockMesh = {0, -2, 0};

    BlockModelElement element = {.from = {0, 0, 0}, .to = {16, 16, 16}};
    element.faces[0] = &(BlockModelElementFaceData){};
    element.faces[2] = &(BlockModelElementFaceData){};
    element.faces[4] = &(BlockModelElementFaceData){};

    uint8_t rotateIndex[6] = {0, 1, 2, 3, 4, 5};
    AddFace(&chunkSubMesh, &blockMesh, 0, 0b010101, &element, rotateIndex, 0, 0, false);

    Mesh mesh;
    createMesh(&mesh, chunkSubMesh.vertices_uv_normal, sizeof(float) * 8 * chunkSubMesh.faceCount * 4,
               chunkSubMesh.indices, sizeof(float) * chunkSubMesh.faceCount * 6);

    cameraPos[0] = 0;
    cameraPos[1] = 0;
    cameraPos[2] = 0;

    mat4x4 model;
    mat4x4_identity_create(model);

    fpsCounterInit();
    int moveSpeed = 10;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_updateViewMatrix();

        vec3 dir = {cameraVec[0], 0, cameraVec[2]}, v = {0, 0, 0};
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
            v[1] = -1;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            v[1] = 1;

        vec3_scale(v, v, deltaTime);
        vec3_add(cameraPos, cameraPos, v);

        // mat4x4_rotate_X(model, model, 0.001);

        // Render model
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        
        glUseProgram(program);
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, (float*)cameraProjectionMat);
        glUniformMatrix4fv(uView, 1, GL_FALSE, (float*)cameraViewMat);
        glUniform3fv(viewPos, 1, cameraPos);

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, 77);
        // glUniform1i(material_diffuse, 0);
        // glUniform1i(material_useDiffuse, 1);

        glUniform3f(dirLight_direction, 0.1, -1, 0.1);
        glUniform4f(dirLight_color, 1, 1, 1, 1);
        glUniform1f(dirLight_ambient, 0.05);
        glUniform1f(dirLight_diffuse, 0.9);

        glUniform4f(material_color, 1, 1, 1, 1);
        glUniform1f(material_shininess, 1);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, (float*)model);
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indicesCount, GL_UNSIGNED_INT, NULL);

        // Render UI
        glDisable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        glDrawString("12345", 0, 20, 20);
        // glDrawString("12345", 0, 0, 0.1);
        // glTextDrawChar('a', -0.99, -0.99, 1.98, 1.98, 0);

        // glUseProgram(glTextDefaultShader);
        // glUniform4f(glTextColor, 1, 1, 1, 1);
        // glTextDrawChar(1, 0, 0, 100, 100, 0);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        frameUpdate();
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}