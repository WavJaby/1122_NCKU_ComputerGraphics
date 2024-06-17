#include "chunk_settings.h"
#include "chunk_sub_mesh.h"
#include "../block/block.h"
#include "../../mesh.h"

#define getBlockIndex(x, y, z) ((x) + ((z) + (y) * CHUNK_SIZE_Z) * CHUNK_SIZE_X)

bool textureIdEquals(void* a, void* b) {
    return *(uint32_t*)a == *(uint32_t*)b;
}
uint32_t textureIdHash(void* a) {
    return *(uint32_t*)a;
}
const NodeInfo info = {
    textureIdEquals,
    textureIdHash,
    NULL,
    0,
};

typedef struct ChunkSub {
    int chunkX, chunkY, chunkZ;
    Block* chunkSubBlocks[CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SUB_Y_SIZE];
    BlockMesh* chunkSubBlockMeshs[CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SUB_Y_SIZE];
    // Map<GLuint TextureId, ChunkSubTextureMesh*>
    Map chunkSubTextureMeshMap;
} ChunkSub;

ChunkSub* chunkSub_new() {
    ChunkSub* chunkSub = calloc(1, sizeof(ChunkSub));
    chunkSub->chunkSubTextureMeshMap.info = info;
    return chunkSub;
}

bool approximately(float a, float b) {
    return abs(b - a) < 1e-06f;
}

bool faceDataSameUV(float arr1[], float arr2[]) {
    if (arr1 == NULL || arr2 == NULL) {
        printf("FaceData uv not found");
        return false;
    }
    return approximately(arr1[0], arr2[0]) &
           approximately(arr1[1], arr2[1]) &
           approximately(arr1[2], arr2[2]) &
           approximately(arr1[3], arr2[3]) &
           approximately(arr1[4], arr2[4]) &
           approximately(arr1[5], arr2[5]) &
           approximately(arr1[6], arr2[6]) &
           approximately(arr1[7], arr2[7]);
}

bool faceDataEquals(BlockModelElementFaceData* faceA, BlockModelElementFaceData* faceB) {
    return faceA->texture.textureId == faceB->texture.textureId &&
           faceDataSameUV(faceA->uv, faceB->uv);
}

ChunkSubTextureMesh* chunkSub_getTextureMesh(ChunkSub* chunkSub, MeshTexture texture) {
    ChunkSubTextureMesh* chunkSubTextureMesh = (ChunkSubTextureMesh*)map_get(&chunkSub->chunkSubTextureMeshMap, &texture.textureId);
    if (chunkSubTextureMesh == NULL) {
        chunkSubTextureMesh = malloc(sizeof(ChunkSubTextureMesh));
        chunkSubTextureMesh->faceCount = 0;
        chunkSubTextureMesh->maxFaceSize = 0;
        chunkSubTextureMesh->indices = NULL;
        chunkSubTextureMesh->vertices_uv_normal = NULL;

        chunkSubTextureMesh->texture = texture;
        chunkSubTextureMesh->mesh = malloc(sizeof(Mesh));

        map_putpp(&chunkSub->chunkSubTextureMeshMap, &chunkSubTextureMesh->texture.textureId, chunkSubTextureMesh);
    }
    return chunkSubTextureMesh;
}

void chunkSub_setBlock(ChunkSub* chunkSub, Block* block) {
    chunkSub->chunkSubBlocks[getBlockIndex(block->xInChunk, block->yInChunk, block->zInChunk)] = block;
}

void chunkSub_loadBlockMesh(ChunkSub* chunkSub, uint8_t* faces, Block* block) {
    BlockMesh blockMesh = {.x = block->xInChunk, .y = block->yInChunk, .z = block->zInChunk};
    uint8_t rotateIndex[6] = {0, 1, 2, 3, 4, 5};

    BlockModel* blockModel = block->model;

    // Load each element in model
    for (size_t i = 0; i < blockModel->elementsLen; i++) {
        BlockModelElement* element = &blockModel->elements[i];

        uint8_t addedFace = 0;
        uint8_t remainingFace = 0b111111;
        while (addedFace < element->faceCount) {
            uint8_t faceMask = 0b000000;
            BlockModelElementFaceData* current = NULL;
            for (uint8_t j = 0; j < 6; j++) {
                if (!element->faces[j] || (remainingFace & (0b1 << j)) == 0) continue;

                // Find new face
                if (!current) {
                    current = element->faces[j];
                    faceMask = (uint8_t)(0b1 << j);
                    addedFace++;
                }
                // Add new face
                else if (faceDataEquals(current, element->faces[j])) {
                    faceMask |= (uint8_t)(0b1 << j);
                    addedFace++;
                }
            }
            if (faceMask == 0b0) continue;
            remainingFace ^= faceMask;
            faceMask &= faces[i];
            if (faceMask == 0b0) continue;

            ChunkSubTextureMesh* textureMesh = chunkSub_getTextureMesh(chunkSub, current->texture);
            // Add face to mesh
            AddFace(textureMesh, &blockMesh, 0, faceMask, element, rotateIndex, 0, 0, false);
            // printf("Add face: %u\n", current->textureId);
        }
    }
}

uint8_t deleteDuplicateFace(BlockModel* model, Block* right, Block* left, Block* top, Block* bottom, Block* front, Block* back, int elementIndex, int modelIndex) {
    uint8_t output = 0b111111;
    uint8_t rotateIndexReverse[] = {0, 1, 2, 3, 4, 5, 6};
    // x
    if (right != NULL && (model != NULL && model->fullBlock && right->model->fullBlock))
        output ^= (uint8_t)(0b1 << rotateIndexReverse[0]);
    // -x
    if (left != NULL && (model != NULL && model->fullBlock && left->model->fullBlock))
        output ^= (uint8_t)(0b1 << rotateIndexReverse[1]);

    // y
    if (top != NULL && (model != NULL && model->fullBlock && top->model->fullBlock))
        output ^= (uint8_t)(0b1 << rotateIndexReverse[2]);
    // -y
    if (bottom != NULL && (model != NULL && model->fullBlock && bottom->model->fullBlock))
        output ^= (uint8_t)(0b1 << rotateIndexReverse[3]);

    // z
    if (front != NULL && (model != NULL && model->fullBlock && front->model->fullBlock))
        output ^= (uint8_t)(0b1 << rotateIndexReverse[4]);
    // -z
    if (back != NULL && (model != NULL && model->fullBlock && back->model->fullBlock))
        output ^= (uint8_t)(0b1 << rotateIndexReverse[5]);
    return output;
}

void chunkSub_initMeshVertices(ChunkSub* chunkSub) {
    for (uint8_t x = 0; x < 16; x++) {
        for (uint8_t y = 0; y < 16; y++) {
            for (uint8_t z = 0; z < 16; z++) {
                Block* block = chunkSub->chunkSubBlocks[getBlockIndex(x, y, z)];
                // Block* right = x == 15 ? (cullRight ? null : rightChunk?._chunkBlock[index]?[0, y, z]) : _chunkBlock[index][x + 1, y, z];
                // Block* left = x == 0 ? (cullLeft ? null : leftChunk?._chunkBlock[index]?[15, y, z]) : _chunkBlock[index][x - 1, y, z];
                // Block* top = y == 15 ? (index == _chunkBlock.Length - 1 ? null : _chunkBlock[index + 1]?[x, 0, z]) : _chunkBlock[index][x, y + 1, z];
                // Block* bottom = y == 0 ? (index == 0 ? null : _chunkBlock[index - 1]?[x, 15, z]) : _chunkBlock[index][x, y - 1, z];
                // Block* front = z == 15 ? (cullFront ? null : frontChunk?._chunkBlock[index]?[x, y, 0]) : _chunkBlock[index][x, y, z + 1];
                // Block* back = z == 0 ? (cullBack ? null : backChunk?._chunkBlock[index]?[x, y, 15]) : _chunkBlock[index][x, y, z - 1];

                Block* right = x == 15 ? NULL : chunkSub->chunkSubBlocks[getBlockIndex(x + 1, y, z)];
                Block* left = x == 0 ? NULL : chunkSub->chunkSubBlocks[getBlockIndex(x - 1, y, z)];
                Block* top = y == 15 ? NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y + 1, z)];
                Block* bottom = y == 0 ? NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y - 1, z)];
                Block* front = z == 15 ? NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y, z + 1)];
                Block* back = z == 0 ? NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y, z - 1)];

                if (!block) continue;
                uint8_t* faces = malloc(sizeof(uint8_t) * block->model->elementsLen);
                for (size_t i = 0; i < block->model->elementsLen; i++) {
                    faces[i] = deleteDuplicateFace(block->model, right, left, top, bottom, front, back, i, -1);
                }
                chunkSub_loadBlockMesh(chunkSub, faces, block);
            }
        }
    }
}

void chunkSub_initMesh(ChunkSub* chunkSub) {
    // Init all mesh
    map_entries(&chunkSub->chunkSubTextureMeshMap, entries) {
        ChunkSubTextureMesh* chunkSubTextureMesh = (ChunkSubTextureMesh*)entries->value;
        createMesh(chunkSubTextureMesh->mesh,
                   chunkSubTextureMesh->vertices_uv_normal, sizeof(float) * chunkSubTextureMesh->faceCount * 32,
                   chunkSubTextureMesh->indices, sizeof(uint32_t) * chunkSubTextureMesh->faceCount * 6);
        printf("Init mesh: %u\n", chunkSubTextureMesh->texture.textureId);
    }
}

void chunkSub_render(ChunkSub* chunkSub, GLint material_color, GLint material_diffuse, GLint material_singleChannel) {
    map_entries(&chunkSub->chunkSubTextureMeshMap, entries) {
        ChunkSubTextureMesh* chunkSubTextureMesh = (ChunkSubTextureMesh*)entries->value;
        Mesh* mesh = chunkSubTextureMesh->mesh;
        // Set texture
        glUniform4fv(material_color, 1, (const GLfloat*)&chunkSubTextureMesh->texture.color);
        glBindTexture(GL_TEXTURE_2D, chunkSubTextureMesh->texture.textureId);
        glUniform1i(material_diffuse, 0);
        glUniform1i(material_singleChannel, chunkSubTextureMesh->texture.singleChannel);

        // Render mesh
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, NULL);
    };
}
