#ifndef __MESH_H__
#define __MESH_H__

typedef struct mesh {
    GLuint vao, vbo, ibo;
    size_t indicesCount;
} Mesh;

void createMesh(Mesh* out, float* vertices, size_t verticesSize, uint32_t* indices, size_t indicesSize) {
    glGenVertexArrays(1, &out->vao);
    glBindVertexArray(out->vao);

    glGenBuffers(2, &out->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, out->vbo);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    out->ibo = out->vbo + 1;
    // glGenBuffers(1, &out->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_STATIC_DRAW);

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

    out->indicesCount = indicesSize >> 2;
}

void deleteMesh(Mesh* mesh) {
    GLuint b[] = {mesh->vbo, mesh->ibo};
    glDeleteBuffers(2, b);
    glDeleteVertexArrays(1, &mesh->vao);
}

#endif