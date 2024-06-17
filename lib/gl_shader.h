#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__

GLuint compileShader(char* src, char* path, GLenum shaderType) {
    // Compile shader
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar* const*)&src, NULL);
    glCompileShader(shader);

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
        return 0;
    }
    return shader;
}

GLuint compileShaderFile(char* path, GLenum shaderType) {
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
    GLuint shader = compileShader(shaderSrc, path, shaderType);
    free(shaderSrc);
    return shader;
}

GLuint compileShaderFileProgram(char* frag, char* vert) {
    GLint fragment_shader = compileShaderFile(frag, GL_FRAGMENT_SHADER);
    GLint vertex_shader = compileShaderFile(vert, GL_VERTEX_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

GLuint compileShaderProgram(char* fragSrc, char* vertSrc) {
    GLint fragment_shader = compileShader(fragSrc, "internal", GL_FRAGMENT_SHADER);
    GLint vertex_shader = compileShader(vertSrc, "internal", GL_VERTEX_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

#endif