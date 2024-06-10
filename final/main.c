#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <stdlib.h>
#include <stdio.h>

#include "linmath.h"
#include "glfw_camera.h"

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
    glViewport(0, 0, width, height);
}

void errorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

double lastMouseX = -1, lastMouseY = -1, mouseSensitivity = 0.6;
void onCursorMove(GLFWwindow* window, double xpos, double ypos) {
    if (lastMouseX == -1) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        return;
    }

    cameraAngle[0] += (ypos - lastMouseY) * mouseSensitivity;
    cameraAngle[1] += (xpos - lastMouseX) * mouseSensitivity;
    printf("%f, %f\n", xpos, ypos);

    lastMouseX = xpos;
    lastMouseY = ypos;
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

uint32_t cubeIndices[] = {
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

GLuint compileShader(char* path, GLenum shaderType) {
    FILE* fileptr = fopen(path, "rb");
    if (!fileptr) {
        printf("Failed to open shader file: %s\n", path);
        return 0;
    }

    fseek(fileptr, 0, SEEK_END);    // Jump to end of file
    long filelen = ftell(fileptr);  // Get current byte offset
    rewind(fileptr);                // Jump back to the beginning
    // Read file
    char* shaderSrc = (char*)malloc(filelen + 1);
    fread(shaderSrc, filelen, 1, fileptr);
    fclose(fileptr);
    shaderSrc[filelen] = '\0';
    // Compile shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar* const*)&shaderSrc, NULL);
    glCompileShader(shader);
    free(shaderSrc);

    // Check if compile faild
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        char* errorLog = malloc(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);
        printf("%s:%s\n", path, errorLog);

        glDeleteShader(shader);
        free(errorLog);
        return -1;
    }
    return shader;
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

    glfwSetKeyCallback(window, onKeyPress);
    glfwSetFramebufferSizeCallback(window, onWindowSizeChange);

    int version = gladLoadGL(glfwGetProcAddress);
    printf("GL Version %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    int triangleTotalSize = sizeof(float) * 8;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, triangleTotalSize, NULL);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, triangleTotalSize, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, triangleTotalSize, (void*)(sizeof(float) * 5));

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLint fragment_shader = compileShader("../shaders/object_shader.frag", GL_FRAGMENT_SHADER);
    GLint vertex_shader = compileShader("../shaders/object_shader.vert", GL_VERTEX_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint uModel = glGetUniformLocation(program, "uModel");
    GLint uView = glGetUniformLocation(program, "uView");
    GLint uProjection = glGetUniformLocation(program, "uProjection");

    GLint viewPos = glGetUniformLocation(program, "viewPos");

    GLint lightMask = glGetUniformLocation(program, "lightMask");
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

    cameraPos[2] = -2;

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // mat4x4_look_at(view, cameraPos, (vec3){0, 0, 0}, (vec3){0, 1, 0});

        camera_updateViewMatrix();

        mat4x4 model;
        mat4x4_identity_create(model);

        glUseProgram(program);
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, (float*)cameraProjectionMat);
        glUniformMatrix4fv(uView, 1, GL_FALSE, (float*)cameraViewMat);
        glUniform3fv(viewPos, 1, cameraPos);

        glUniform1ui(lightMask, 0b00000001);
        glUniform3f(dirLight_direction, 0, -1, 1);
        glUniform4f(dirLight_color, 1, 1, 1, 1);
        glUniform1f(dirLight_ambient, 0.1);
        glUniform1f(dirLight_diffuse, 0.5);

        glUniform4f(material_color, 1, 1, 1, 1);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, (float*)model);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) >> 2, GL_UNSIGNED_INT, NULL);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}