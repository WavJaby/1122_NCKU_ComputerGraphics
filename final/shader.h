#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <glad/gl.h>

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

        char* errorLog = (char*)malloc(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, errorLog);
        printf("%s:%s\n", path, errorLog);

        glDeleteShader(shader);
        free(errorLog);
        return -1;
    }
    return shader;
}

GLuint compileShaderProgram(char* frag, char* vert) {
    GLint fragment_shader = compileShader(frag, GL_FRAGMENT_SHADER);
    GLint vertex_shader = compileShader(vert, GL_VERTEX_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}