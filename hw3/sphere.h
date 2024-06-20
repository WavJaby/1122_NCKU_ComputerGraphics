
#include <stdlib.h>
#include <GL/glew.h>

int sphere_setupVBO(GLuint vboIds[4], float radius, int slices, int stacks) {
    int numVertices = (stacks + 1) * (slices + 1);
    int numIndices = stacks * slices * 6;

    float *vertices = malloc(sizeof(float) * 3 * numVertices);
    float *normals = malloc(sizeof(float) * 3 * numVertices);
    float *texCoords = malloc(sizeof(float) * 2 * numVertices);
    unsigned int *indices = malloc(sizeof(unsigned int) * numIndices);

    int vertexIdx = 0, normalsIdx = 0, texIdx = 0;
    for (int i = 0; i <= stacks; ++i) {
        float V = (float)i / (float)stacks;
        float phi = V * M_PI;

        for (int j = 0; j <= slices; ++j) {
            float U = (float)j / (float)slices;
            float theta = U * 2.0 * M_PI;

            float x = cosf(theta) * sinf(phi);
            float y = cosf(phi);
            float z = sinf(theta) * sinf(phi);

            // Vertex position
            vertices[vertexIdx++] = x * radius;
            vertices[vertexIdx++] = y * radius;
            vertices[vertexIdx++] = z * radius;

            // Normal vectors
            normals[normalsIdx++] = x;
            normals[normalsIdx++] = y;
            normals[normalsIdx++] = z;

            // Texture coordinates
            texCoords[texIdx++] = U;
            texCoords[texIdx++] = V;
        }
    }

    int idx = 0;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = (i * (slices + 1)) + j;
            int second = first + slices + 1;

            indices[idx++] = first + 1;
            indices[idx++] = second;
            indices[idx++] = first;

            indices[idx++] = first + 1;
            indices[idx++] = second + 1;
            indices[idx++] = second;
        }
    }

    glGenBuffers(4, vboIds);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * (stacks + 1) * (slices + 1) * sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * (stacks + 1) * (slices + 1) * sizeof(float), normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    glBufferData(GL_ARRAY_BUFFER, 2 * (stacks + 1) * (slices + 1) * sizeof(float), texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    free(vertices);
    free(normals);
    free(texCoords);
    free(indices);

    return numIndices;
}

void sphere_render(GLuint vboIds[3], int numIndices) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[1]);
    glNormalPointer(GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vboIds[2]);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[3]);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}