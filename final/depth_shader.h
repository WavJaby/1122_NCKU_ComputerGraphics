typedef struct DepthShader {
    GLuint program;
    GLint
        uModel,
        uLlightSpaceMatrix;
} DepthShader;
DepthShader depthShader = {};

void depthShaderInit() {
    GLuint program = depthShader.program = compileShaderFileProgram("../shaders/depth_shader.frag", "../shaders/depth_shader.vert");

    depthShader.uModel = glGetUniformLocation(program, "uModel");
    depthShader.uLlightSpaceMatrix = glGetUniformLocation(program, "uLlightSpaceMatrix");
}