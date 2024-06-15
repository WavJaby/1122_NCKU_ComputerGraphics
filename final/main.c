#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "linmath.h"
#include "glfw_camera.h"
#include "fps_counter.h"
#include "mesh.h"
#include "shader.h"

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

typedef struct BlockModelElementFaceData {
    int testureId;
    // [x,y] * 4
    float uv[8];
} BlockModelElementFaceData;

typedef struct BlockModelElement {
    vec3 from, to;
    // x,-x, y,-y, z,-z
    BlockModelElementFaceData* faces[6];
    uint8_t cullFaces[6];  // cull face index
} BlockModelElement;

typedef struct BlockModel {
    BlockModelElement* elements;
    int elementsLen;
} BlockModel;

typedef struct BlockMesh {
    int x, y, z;
} BlockMesh;

typedef struct ChunkSubMesh {
    // x,y,z, u,v, x,y,z
    float* vertices_uv_normal;
    uint32_t* indices;
    // 1 face contains 2 triangle
    int faceCount;
} ChunkSubMesh;

#define setVertexData(vertices, index, \
                      u, v,            \
                      x, y, z,         \
                      nx, ny, nz)      \
    vertices[index] = x;               \
    vertices[index + 1] = y;           \
    vertices[index + 2] = z;           \
    vertices[index + 3] = u;           \
    vertices[index + 4] = v;           \
    vertices[index + 5] = nx;          \
    vertices[index + 6] = ny;          \
    vertices[index + 7] = nz;          \
    index += 8;

/**
 * @brief
 *
 * @param chunkSubMesh target mesh to render
 * @param blockMesh rendered block model infomation
 * @param elementIndex element index in model
 * @param face 6 bytes, -z,z,-y,y,-x,x
 * @param element block model element to render
 * @param rotateIndex face index after rotation
 * @param rotX block rotation x
 * @param rotY block rotation y
 * @param uvLock local texture rotation on +y, -y face
 */
void AddFace(ChunkSubMesh* chunkSubMesh, BlockMesh* blockMesh, int elementIndex,
             uint8_t face, BlockModelElement* element,
             uint8_t rotateIndex[6],
             int rotX, int rotY, bool uvLock) {
    uint8_t newFaceCount = (uint8_t)((face & 0b1) + (face >> 1 & 0b1) + (face >> 2 & 0b1) + (face >> 3 & 0b1) + (face >> 4 & 0b1) + (face >> 5 & 0b1));
    if (newFaceCount == 0) return;

    // if ((_faceCount + newFaceCount) * 4 > _vertices.Length) {
    //     int size = (int)(_faceCount * 1.5f) * 4;
    //     Array.Resize(ref _vertices, size + 1);
    //     Array.Resize(ref _uv, size + 1);
    // }

    // if (_faceCount + newFaceCount > _faceIndex.Length)
    //     Array.Resize(ref _faceIndex, (int)(_faceIndex.Length * 1.5f));
    // if ((_faceCount + newFaceCount) * 6 > _indices.Length)
    //     Array.Resize(ref _indices, (int)(_faceCount * 1.5f) * 6);

    // int triCnt = _faceCount * 6;

    // 到目前所有的頂點
    // int cnt = _faceCount * 4 + 1;

    uint32_t* _indices = chunkSubMesh->indices;
    float* _vertices_uv_normal = chunkSubMesh->vertices_uv_normal;

    // total indices count
    int indicesLen = chunkSubMesh->faceCount * 6;
    int vertexIndex = chunkSubMesh->faceCount * 4;
    // total vertices float array count
    int verticesSize = chunkSubMesh->faceCount * 8 * 4;

    float _modelScale = 1.f / 16;
    float from[3] = {element->from[0] * _modelScale, element->from[1] * _modelScale, 1 - element->to[2] * _modelScale};
    float to__[3] = {element->to[0] * _modelScale, element->to[1] * _modelScale, 1 - element->from[2] * _modelScale};

    // Vector3 center;
    // Vector3 blockCenter;
    // Quaternion rotation;
    // Quaternion blockRotation;
    // bool rotate, blockRotate = rotX != 0 || rotY != 0;
    // if (blockRotate) {
    //     blockCenter = new Vector3(
    //         _defaultBlockCenter.x + blockMesh->x,
    //         _defaultBlockCenter.y + blockMesh->y,
    //         _defaultBlockCenter.z + blockMesh->z);
    //     blockRotation = Quaternion.Euler(rotX, rotY, 0);
    // } else {
    //     blockCenter = default;
    //     blockRotation = default;
    // }
    // if (element->Rotation != null) {
    //     center = new Vector3(
    //         blockMesh->x + element->Rotation.Origin[0] / _textureSize,
    //         blockMesh->y + element->Rotation.Origin[1] / _textureSize,
    //         blockMesh->z + 1f - element->Rotation.Origin[2] / _textureSize);
    //     rotation = Quaternion.Euler(-element->Rotation.Angle[0], element->Rotation.Angle[1], element->Rotation.Angle[2]);
    //     rotate = element->Rotation.Angle[0] != 0 || element->Rotation.Angle[1] != 0 || element->Rotation.Angle[2] != 0;
    //     // Debug.Log($"{rotateX},{rotateY}, {rotate}");
    // } else {
    //     center = default;
    //     rotation = default;
    //     rotate = false;
    // }

    // x
    if ((face & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[0];
        if (!faceData) {
            fprintf(stderr, "FaceData +x is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[0]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float x = blockMesh->x + to__[0];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], x, blockMesh->y + from[1], blockMesh->z + from[2], 1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], x, blockMesh->y + to__[1], blockMesh->z + from[2], 1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], x, blockMesh->y + to__[1], blockMesh->z + to__[2], 1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], x, blockMesh->y + from[1], blockMesh->z + to__[2], 1, 0, 0);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // -x
    if ((face >> 1 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[1];
        if (!faceData) {
            fprintf(stderr, "FaceData -x is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[1]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float x = blockMesh->x + from[0];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], x, blockMesh->y + from[1], blockMesh->z + to__[2], -1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], x, blockMesh->y + to__[1], blockMesh->z + to__[2], -1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], x, blockMesh->y + to__[1], blockMesh->z + from[2], -1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], x, blockMesh->y + from[1], blockMesh->z + from[2], -1, 0, 0);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // y
    if ((face >> 2 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[2];
        if (!faceData) {
            fprintf(stderr, "FaceData +y is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[2]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float y = blockMesh->y + to__[1];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + from[0], y, blockMesh->z + from[2], 0, 1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + from[0], y, blockMesh->z + to__[2], 0, 1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + to__[0], y, blockMesh->z + to__[2], 0, 1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + to__[0], y, blockMesh->z + from[2], 0, 1, 0);
        // if (uvLock)
        //     RotateTexture(cnt, -rotY);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // -y
    if ((face >> 3 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[3];
        if (!faceData) {
            fprintf(stderr, "FaceData -y is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[3]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float y = blockMesh->y + from[1];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + to__[0], y, blockMesh->z + from[2], 0, -1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + to__[0], y, blockMesh->z + to__[2], 0, -1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + from[0], y, blockMesh->z + to__[2], 0, -1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + from[0], y, blockMesh->z + from[2], 0, -1, 0);
        // if (uvLock)
        //     RotateTexture(cnt, -rotY);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // z
    if ((face >> 4 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[4];
        if (!faceData) {
            fprintf(stderr, "FaceData +z is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[4]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float z = blockMesh->z + to__[2];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + to__[0], blockMesh->y + from[1], z, 0, 0, 1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + to__[0], blockMesh->y + to__[1], z, 0, 0, 1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + from[0], blockMesh->y + to__[1], z, 0, 0, 1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + from[0], blockMesh->y + from[1], z, 0, 0, 1);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // -z
    if ((face >> 5 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[5];
        if (!faceData) {
            fprintf(stderr, "FaceData -z is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[5]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen] = vertexIndex + 3;
        float z = blockMesh->z + from[2];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + from[0], blockMesh->y + from[1], z, 0, 0, -1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + from[0], blockMesh->y + to__[1], z, 0, 0, -1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + to__[0], blockMesh->y + to__[1], z, 0, 0, -1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + to__[0], blockMesh->y + from[1], z, 0, 0, -1);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }
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
    // Windows dont fire FramebufferSizeCallback at start
    camera_windowSizeUpdate(windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window, onWindowSizeChange);
    glfwSetKeyCallback(window, onKeyPress);

    // Init context setting
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        printf("Raw mouse motion support\n");
    }
    glfwSetCursorPosCallback(window, onCursorMove);

    int version = gladLoadGL(glfwGetProcAddress);
    printf("GL Version %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    fpsCounterInit();

    GLuint program = compileShaderProgram("../shaders/object_shader.frag", "../shaders/object_shader.vert");

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
    BlockMesh blockMesh = {0, 0, 0};

    BlockModelElement element = {.from = {0, 0, 0}, .to = {16, 16, 16}};
    element.faces[0] = &(BlockModelElementFaceData){};
    element.faces[2] = &(BlockModelElementFaceData){};
    element.faces[4] = &(BlockModelElementFaceData){};

    uint8_t rotateIndex[6] = {0, 1, 2, 3, 4, 5};
    AddFace(&chunkSubMesh, &blockMesh, 0, 0b010101, &element, rotateIndex, 0, 0, false);

    Mesh mesh;
    // createMesh(&mesh, cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices));
    createMesh(&mesh, chunkSubMesh.vertices_uv_normal, sizeof(float) * 8 * chunkSubMesh.faceCount * 4,
               chunkSubMesh.indices, sizeof(float) * chunkSubMesh.faceCount * 6);

    cameraPos[0] = 0;
    cameraPos[1] = 0;
    cameraPos[2] = 0;

    fpsCounterInit();

    mat4x4 model;
    mat4x4_identity_create(model);

    int moveSpeed = 7;
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
            cameraPos[1] -= moveSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos[1] += moveSpeed * deltaTime;

        vec3_scale(v, v, deltaTime);
        vec3_add(cameraPos, cameraPos, v);

        // mat4x4_rotate_X(model, model, 0.001);

        glUseProgram(program);
        glUniformMatrix4fv(uProjection, 1, GL_FALSE, (float*)cameraProjectionMat);
        glUniformMatrix4fv(uView, 1, GL_FALSE, (float*)cameraViewMat);
        glUniform3fv(viewPos, 1, cameraPos);

        glUniform3f(dirLight_direction, 0, -1, 0);
        glUniform4f(dirLight_color, 1, 1, 1, 1);
        glUniform1f(dirLight_ambient, 0.05);
        glUniform1f(dirLight_diffuse, 0.9);

        glUniform4f(material_color, 1, 1, 1, 1);
        glUniform1f(material_shininess, 1);
        glUniformMatrix4fv(uModel, 1, GL_FALSE, (float*)model);
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indicesCount, GL_UNSIGNED_INT, NULL);

        glfwSwapBuffers(window);
        frameUpdate();
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}