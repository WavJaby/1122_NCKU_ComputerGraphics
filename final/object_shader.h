typedef struct ObjectShader {
    GLuint program;
    GLint
        uModel,
        uView,
        uProjection,
        uLightSpaceMatrix,
        uShadowMap,
        uViewPos,
        dirLight_direction,
        dirLight_color,
        dirLight_ambient,
        dirLight_diffuse,
        material_color,
        material_useDiffuse,
        material_diffuse,
        material_singleChannel,
        material_useSpecular,
        material_specular,
        material_shininess;
} ObjectShader;
ObjectShader objectShader = {};

void objectShaderInit() {
    GLuint program = objectShader.program = compileShaderFileProgram("../shaders/object_shader.frag", "../shaders/object_shader.vert");

    objectShader.uModel = glGetUniformLocation(program, "uModel");
    objectShader.uView = glGetUniformLocation(program, "uView");
    objectShader.uProjection = glGetUniformLocation(program, "uProjection");
    objectShader.uLightSpaceMatrix = glGetUniformLocation(program, "uLightSpaceMatrix");
    objectShader.uShadowMap = glGetUniformLocation(program, "uShadowMap");
    objectShader.uViewPos = glGetUniformLocation(program, "uViewPos");

    objectShader.dirLight_direction = glGetUniformLocation(program, "dirLight.direction");
    objectShader.dirLight_color = glGetUniformLocation(program, "dirLight.base.color");
    objectShader.dirLight_ambient = glGetUniformLocation(program, "dirLight.base.ambientIntensity");
    objectShader.dirLight_diffuse = glGetUniformLocation(program, "dirLight.base.diffuseIntensity");

    objectShader.material_color = glGetUniformLocation(program, "material.color");
    objectShader.material_useDiffuse = glGetUniformLocation(program, "material.useDiffuse");
    objectShader.material_diffuse = glGetUniformLocation(program, "material.diffuse");
    objectShader.material_singleChannel = glGetUniformLocation(program, "material.singleChannel");
    objectShader.material_useSpecular = glGetUniformLocation(program, "material.useSpecular");
    objectShader.material_specular = glGetUniformLocation(program, "material.specular");
    objectShader.material_shininess = glGetUniformLocation(program, "material.shininess");
}