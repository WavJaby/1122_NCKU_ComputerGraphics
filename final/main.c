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

#include "linmath.h"
#include "glfw_camera.h"
#include "fps_counter.h"
#include "mesh.h"
#include "gl_shader.h"
#include "gl_text.h"
#include "game/chunk/chunk_sub.h"
#include "game/chunk/chunk.h"
#include "game/block/block.h"

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

char fpsInfo[64];
void fpsUpdate(float fps, float tick) {
    sprintf(fpsInfo, "fps:%7.2f, tick: %.2f, d: %.5f", fps, tick, deltaTime);
}

void loadTextureImage(MeshTexture* texture, char* texturePath) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
    if (data == NULL) {
        printf("failed to load texture: %s\n", texturePath);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (nrChannels == 1) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    } else if (nrChannels == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else if (nrChannels == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    texture->textureId = textureID;
    texture->singleChannel = nrChannels == 1;
}

float terrain(int x, int y, float size) {
    float val = 0;

    float freq = 1;
    float amp = 1;

    // Sampling
    for (int i = 0; i < 3; i++) {
        val += perlin(x * freq / size, y * freq / size) * amp;

        freq *= 2;
        amp /= 2;
    }

    // Contrast
    val *= 1.2;

    // Clipping
    if (val > 1.0f)
        val = 1.0f;
    else if (val < -1.0f)
        val = -1.0f;

    // Normalize
    val = (val + 1) * 0.5f;
    return val;
}

typedef struct ChunkCoord {
    int x, z;
} ChunkCoord;

bool chunkCoordEquals(void* a, void* b) {
    return ((ChunkCoord*)a)->x == ((ChunkCoord*)b)->x &&
           ((ChunkCoord*)a)->z == ((ChunkCoord*)b)->z;
}
uint32_t chunkCoordHash(void* a) {
    return ((ChunkCoord*)a)->x + ((ChunkCoord*)a)->z << 16;
}
const NodeInfo chunkCoordInfo = {
    chunkCoordEquals,
    chunkCoordHash,
    NULL,
    WJCL_HASH_MAP_FREE_KEY,
};
// Map<ChunkCoord*, ChunkSub*>
Map chunks = map_create(chunkCoordInfo);

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
    GLint material_singleChannel = glGetUniformLocation(program, "material.singleChannel");
    GLint material_useSpecular = glGetUniformLocation(program, "material.useSpecular");
    GLint material_specular = glGetUniformLocation(program, "material.specular");
    GLint material_shininess = glGetUniformLocation(program, "material.shininess");

    MeshTexture dirt = {.color = {1, 1, 1, 1}};
    loadTextureImage(&dirt, "../minecraft/textures/block/dirt.png");

    MeshTexture grass_block_top = {.color = {99 / 255.f, 185 / 255.f, 39 / 255.f, 1}};
    loadTextureImage(&grass_block_top, "../minecraft/textures/block/grass_block_top.png");

    MeshTexture oak_log = {.color = {1, 1, 1, 1}};
    loadTextureImage(&oak_log, "../minecraft/textures/block/oak_log.png");

    MeshTexture oak_log_top = {.color = {1, 1, 1, 1}};
    loadTextureImage(&oak_log_top, "../minecraft/textures/block/oak_log_top.png");

    int modelElementCount = 1;
    BlockModel blockModel = {malloc(sizeof(BlockModelElement) * modelElementCount), modelElementCount};
    blockModel.fullBlock = true;

    BlockModelElement* element = &blockModel.elements[0];
    vec3_dup(element->from, (vec3){0, 0, 0});
    vec3_dup(element->to, (vec3){16, 16, 16});
    element->faceCount = 6;
    element->faces[0] = &(BlockModelElementFaceData){.uv = {0, 0, 0, 1, 1, 1, 1, 0}, .texture = dirt};
    element->faces[1] = &(BlockModelElementFaceData){.uv = {0, 0, 0, 1, 1, 1, 1, 0}, .texture = dirt};
    element->faces[2] = &(BlockModelElementFaceData){.uv = {0, 0, 0, 1, 1, 1, 1, 0}, .texture = grass_block_top};
    element->faces[3] = &(BlockModelElementFaceData){.uv = {0, 0, 0, 1, 1, 1, 1, 0}, .texture = dirt};
    element->faces[4] = &(BlockModelElementFaceData){.uv = {0, 0, 0, 1, 1, 1, 1, 0}, .texture = dirt};
    element->faces[5] = &(BlockModelElementFaceData){.uv = {0, 0, 0, 1, 1, 1, 1, 0}, .texture = dirt};

    for (int cx = -21; cx < 21; cx++) {
        for (int cz = -21; cz < 21; cz++) {
            ChunkCoord* coord = malloc(sizeof(ChunkCoord));
            coord->x = cx;
            coord->z = cz;
            ChunkSub* chunkSub = chunkSub_new();
            chunkSub->chunkX = cx;
            chunkSub->chunkZ = cz;
            chunkSub->chunkY = 0;
            map_putpp(&chunks, coord, chunkSub);

            ChunkCoord coordNext = {cx - 1, cz};
            ChunkSub* left = (ChunkSub*)map_get(&chunks, &coordNext);
            if (left) {
                left->right = chunkSub;
                chunkSub->left = left;
            }
            coordNext.x = cx + 1;
            ChunkSub* right = (ChunkSub*)map_get(&chunks, &coordNext);
            if (right) {
                right->left = chunkSub;
                chunkSub->right = right;
            }
            coordNext.x = cx;
            coordNext.z = cz + 1;
            ChunkSub* front = (ChunkSub*)map_get(&chunks, &coordNext);
            if (front) {
                front->back = chunkSub;
                chunkSub->front = front;
            }
            coordNext.z = cz - 1;
            ChunkSub* back = (ChunkSub*)map_get(&chunks, &coordNext);
            if (back) {
                back->front = chunkSub;
                chunkSub->back = back;
            }

            // Create chunk blocks
            int xOff = cx * CHUNK_SIZE_X + 65535, zOff = cz * CHUNK_SIZE_Z + 65535;
            for (uint8_t x = 0; x < 16; x++) {
                for (uint8_t z = 0; z < 16; z++) {
                    int h = terrain(xOff + x, zOff + z, 100) * 16 + 1;

                    for (uint8_t y = 0; y < h; y++) {
                        Block* block = (Block*)malloc(sizeof(Block));
                        block->xInChunk = x;
                        block->yInChunk = y;
                        block->zInChunk = z;
                        block->model = &blockModel;
                        chunkSub_setBlock(chunkSub, block);
                    }
                }
            }
        }
    }

    // Init chunk mesh
    for (int cx = -20; cx < 20; cx++) {
        for (int cz = -20; cz < 20; cz++) {
            ChunkCoord coordNext = {cx, cz};
            ChunkSub* chunkSub = (ChunkSub*)map_get(&chunks, &coordNext);
            chunkSub_initMeshVertices(chunkSub);
            chunkSub_initMesh(chunkSub);
        }
    }

    cameraPos[0] = 0;
    cameraPos[1] = 2;
    cameraPos[2] = 0;

    mat4x4 model;

    fpsCounterInit();
    int moveSpeed = 50;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_updateViewMatrix();

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

        glUseProgram(program);
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, (float*)cameraProjectionMat);
        glUniformMatrix4fv(uView, 1, GL_FALSE, (float*)cameraViewMat);
        glUniform3fv(viewPos, 1, cameraPos);

        vec3 lightDir = {0.5, -1, 0.5};
        vec3_norm(lightDir, lightDir);
        glUniform3fv(dirLight_direction, 1, (const GLfloat*)lightDir);
        glUniform4f(dirLight_color, 1, 1, 1, 1);
        glUniform1f(dirLight_ambient, 0.1);
        glUniform1f(dirLight_diffuse, 0.8);

        glUniform1f(material_shininess, 0);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(material_useDiffuse, 1);

        // Render chunks
        map_entries(&chunks, entries) {
            ChunkSub* chunkSub = (ChunkSub*)entries->value;
            ChunkCoord* chunkCoord = (ChunkCoord*)entries->key;
            mat4x4_translate_create(model, chunkCoord->x * CHUNK_SIZE_X, 0, chunkCoord->z * CHUNK_SIZE_Z);
            glUniformMatrix4fv(uModel, 1, GL_FALSE, (float*)model);

            chunkSub_render(chunkSub, material_color, material_diffuse, material_singleChannel);
        }

        // Render UI
        glDisable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        ui_textDrawString(fpsInfo, 5, 20, 20);
        char infoCache[128];
        sprintf(infoCache, "Camera pos: (%.2f, %.2f, %.2f), dir: (%.2f, %.2f, %.2f)",
                cameraPos[0], cameraPos[1], cameraPos[2],
                cameraDir[0], cameraDir[1], cameraDir[2]);
        ui_textDrawString(infoCache, 5, 20 * 2, 20);

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        frameUpdate(fpsUpdate);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}