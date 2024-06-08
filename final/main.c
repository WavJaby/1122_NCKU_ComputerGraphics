#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <stdlib.h>
#include <stdio.h>

int windowWidth = 640;
int windowHeight = 480;
char windowTitle[] = "Final Project";

static void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void onWindowSizeChange(GLFWwindow* window, int width, int height) {
    printf("%dx%d\n", width, height);
    glViewport(0, 0, width, height);
}

void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

float cubeVertices[] = {
    // X    Y      Z      UV          Normal
    // Front face
    -0.5f, -0.5f, +0.5f, 0.0f, 1.0f, 0.0f, 0.0f, +1.0f,
    +0.5f, -0.5f, +0.5f, 1.0f, 1.0f, 0.0f, 0.0f, +1.0f,
    +0.5f, +0.5f, +0.5f, 1.0f, 0.0f, 0.0f, 0.0f, +1.0f,
    -0.5f, +0.5f, +0.5f, 0.0f, 0.0f, 0.0f, 0.0f, +1.0f,
    // Back face
    -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
    +0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
    +0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    -0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    // Right face
    +0.5f, -0.5f, +0.5f, 0.0f, 1.0f, +1.0f, 0.0f, 0.0f,
    +0.5f, -0.5f, -0.5f, 1.0f, 1.0f, +1.0f, 0.0f, 0.0f,
    +0.5f, +0.5f, -0.5f, 1.0f, 0.0f, +1.0f, 0.0f, 0.0f,
    +0.5f, +0.5f, +0.5f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
    // Left face
    -0.5f, -0.5f, +0.5f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, +0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -0.5f, +0.5f, +0.5f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    // Top face
    -0.5f, +0.5f, +0.5f, 0.0f, 1.0f, 0.0f, +1.0f, 0.0f,
    +0.5f, +0.5f, +0.5f, 1.0f, 1.0f, 0.0f, +1.0f, 0.0f,
    +0.5f, +0.5f, -0.5f, 1.0f, 0.0f, 0.0f, +1.0f, 0.0f,
    -0.5f, +0.5f, -0.5f, 0.0f, 0.0f, 0.0f, +1.0f, 0.0f,
    // Bottom face
    -0.5f, -0.5f, +0.5f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    +0.5f, -0.5f, +0.5f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
    +0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    //
};

int cubeIndices[] = {
    // Front face
    0, 1, 2,
    2, 3, 0,
    // Back face
    4, 5, 6,
    6, 7, 4,
    // Right face
    8, 9, 10,
    10, 11, 8,
    // Left face
    12, 13, 14,
    14, 15, 12,
    // Top face
    16, 17, 18,
    18, 19, 16,
    // Bottom face
    20, 21, 22,
    22, 23, 20,
    //
};

int main(int argc, char* argv[]) {
    printf("Program start\n");

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    // Set minimum version required
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    printf("GL Version %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glfwSetKeyCallback(window, onKeyPress);
    glfwSetFramebufferSizeCallback(window, onWindowSizeChange);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertex_buffer;
    glGenBuffers(2, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer + 1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer
    int vertexSize = 8;
    glVertexAttribPointer(0, 3, GL_FLOAT, vertexSize, 0, NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, vertexSize, 3, NULL);
    glVertexAttribPointer(2, 3, GL_FLOAT, vertexSize, 5, NULL);

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}