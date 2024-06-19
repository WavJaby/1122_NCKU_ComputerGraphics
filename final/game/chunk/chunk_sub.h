#ifndef __CHUNK_SUB_H__
#define __CHUNK_SUB_H__

#include "chunk_settings.h"
#include "chunk_sub_mesh.h"
#include "../block/block.h"
#include "chunk.h"

#define getBlockIndex(x, y, z) ((x) + ((z) + (y) * CHUNK_SIZE_Z) * CHUNK_SIZE_X)

bool textureIdEquals(void* a, void* b) {
    return *(uint32_t*)a == *(uint32_t*)b;
}
uint32_t textureIdHash(void* a) {
    return *(uint32_t*)a;
}
const MapNodeInfo info = {
    textureIdEquals,
    textureIdHash,
    NULL,
    0,
};

typedef enum ChunkSubState {
    CHUNK_SUB_EMPTY,
    CHUNK_SUB_BLOCK_LOADED,
    CHUNK_SUB_MESH_CREATED,
} ChunkSubState;

typedef struct ChunkSub {
    int subIndex;
    Block* chunkSubBlocks[CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SUB_Y_SIZE];
    BlockMesh* chunkSubBlockMeshs[CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SUB_Y_SIZE];
    ChunkSubState state;
    // Map<GLuint TextureId, ChunkSubTextureMesh*>
    Map chunkSubTextureMeshMap;

    struct Chunk* parent;
} ChunkSub;

#define chunkSub_top(chunkSub) (chunkSub->subIndex + 1 == TOTAL_CHUNK_SUB_HEIGHT ? NULL : chunkSub->parent->chunkSub[chunkSub->subIndex + 1])
#define chunkSub_bottom(chunkSub) (chunkSub->subIndex == 0 ? NULL : chunkSub->parent->chunkSub[chunkSub->subIndex - 1])

ChunkSub* chunkSub_new(Chunk* parent, int blockY) {
    ChunkSub* chunkSub = calloc(1, sizeof(ChunkSub));
    chunkSub->state = CHUNK_SUB_EMPTY;
    chunkSub->chunkSubTextureMeshMap.info = info;

    chunkSub->parent = parent;
    chunkSub->subIndex = chunk_getChunkSubIndexByBlockY(blockY);
    return chunkSub;
}

ChunkSub* chunkSub_newWithIndex(Chunk* parent, int index) {
    ChunkSub* chunkSub = calloc(1, sizeof(ChunkSub));
    chunkSub->state = CHUNK_SUB_EMPTY;
    chunkSub->chunkSubTextureMeshMap.info = info;

    chunkSub->parent = parent;
    chunkSub->subIndex = index;
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

ChunkSubTextureMesh* chunkSub_getTextureMesh(ChunkSub* chunkSub, Texture texture) {
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

Block* chunkSub_getBlock(ChunkSub* chunkSub, uint8_t xInChunk, uint8_t yInChunk, uint8_t zInChunk) {
    return chunkSub->chunkSubBlocks[getBlockIndex(xInChunk, yInChunk, zInChunk)];
}

void chunkSub_loadBlockMesh(ChunkSub* chunkSub, uint8_t* faces, Block* block) {
    BlockMesh blockMesh = {.x = block->xInChunk, .y = block->yInChunk, .z = block->zInChunk};
    uint8_t rotateIndex[6] = {0, 1, 2, 3, 4, 5};

    BlockModel* blockModel = block->model;

    // Load each element in model
    for (size_t i = 0; i < blockModel->elementsCount; i++) {
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

static inline bool chunkSub_isReadyInitMesh(ChunkSub* chunkSub) {
    ChunkSub* leftChunkSub = chunkSub->parent->left ? chunkSub->parent->left->chunkSub[chunkSub->subIndex] : NULL;
    ChunkSub* rightChunkSub = chunkSub->parent->right ? chunkSub->parent->right->chunkSub[chunkSub->subIndex] : NULL;
    ChunkSub* topChunkSub = chunkSub_top(chunkSub);
    ChunkSub* bottomChunkSub = chunkSub_bottom(chunkSub);
    ChunkSub* frontChunkSub = chunkSub->parent->front ? chunkSub->parent->front->chunkSub[chunkSub->subIndex] : NULL;
    ChunkSub* backChunkSub = chunkSub->parent->back ? chunkSub->parent->back->chunkSub[chunkSub->subIndex] : NULL;
    return chunkSub->state == CHUNK_SUB_BLOCK_LOADED &&
           leftChunkSub && leftChunkSub->state &&
           rightChunkSub && rightChunkSub->state &&
           topChunkSub && topChunkSub->state &&
           bottomChunkSub && bottomChunkSub->state &&
           frontChunkSub && frontChunkSub->state &&
           backChunkSub && backChunkSub->state;
}

void chunkSub_initMeshVertices(ChunkSub* chunkSub) {
    for (uint8_t x = 0; x < 16; x++) {
        for (uint8_t y = 0; y < 16; y++) {
            for (uint8_t z = 0; z < 16; z++) {
                Block* block = chunkSub->chunkSubBlocks[getBlockIndex(x, y, z)];
                if (!block) continue;
                ChunkSub* rightChunkSub = chunkSub->parent->right ? chunkSub->parent->right->chunkSub[chunkSub->subIndex] : NULL;
                Block* right = x == 15 ? rightChunkSub ? rightChunkSub->chunkSubBlocks[getBlockIndex(0, y, z)] : NULL : chunkSub->chunkSubBlocks[getBlockIndex(x + 1, y, z)];

                ChunkSub* leftChunkSub = chunkSub->parent->left ? chunkSub->parent->left->chunkSub[chunkSub->subIndex] : NULL;
                Block* left = x == 0 ? leftChunkSub ? leftChunkSub->chunkSubBlocks[getBlockIndex(CHUNK_SIZE_X - 1, y, z)] : NULL : chunkSub->chunkSubBlocks[getBlockIndex(x - 1, y, z)];

                ChunkSub* topChunkSub = chunkSub_top(chunkSub);
                Block* top = y == 15 ? topChunkSub ? topChunkSub->chunkSubBlocks[getBlockIndex(x, 0, z)] : NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y + 1, z)];

                ChunkSub* bottomChunkSub = chunkSub_bottom(chunkSub);
                Block* bottom = y == 0 ? bottomChunkSub ? bottomChunkSub->chunkSubBlocks[getBlockIndex(x, CHUNK_SUB_Y_SIZE - 1, z)] : NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y - 1, z)];

                ChunkSub* frontChunkSub = chunkSub->parent->front ? chunkSub->parent->front->chunkSub[chunkSub->subIndex] : NULL;
                Block* front = z == 15 ? frontChunkSub ? frontChunkSub->chunkSubBlocks[getBlockIndex(x, y, 0)] : NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y, z + 1)];

                ChunkSub* backChunkSub = chunkSub->parent->back ? chunkSub->parent->back->chunkSub[chunkSub->subIndex] : NULL;
                Block* back = z == 0 ? backChunkSub ? backChunkSub->chunkSubBlocks[getBlockIndex(x, y, CHUNK_SIZE_Z - 1)] : NULL : chunkSub->chunkSubBlocks[getBlockIndex(x, y, z - 1)];

                uint8_t* faces = malloc(sizeof(uint8_t) * block->model->elementsCount);
                for (size_t i = 0; i < block->model->elementsCount; i++) {
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
        // printf("Init mesh: %u\n", chunkSubTextureMesh->texture.textureId);
    }
}

void chunkSub_render(ChunkSub* chunkSub, bool renderShadow) {
    map_entries(&chunkSub->chunkSubTextureMeshMap, entries) {
        ChunkSubTextureMesh* chunkSubTextureMesh = (ChunkSubTextureMesh*)entries->value;
        Mesh* mesh = chunkSubTextureMesh->mesh;
        // Apply texture if normal render
        if (!renderShadow) {
            // Set texture
            glUniform4fv(objectShader.material_color, 1, (const GLfloat*)&chunkSubTextureMesh->texture.color);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, chunkSubTextureMesh->texture.textureId);
            glUniform1i(objectShader.material_diffuse, 0);
            glUniform1i(objectShader.material_singleChannel, chunkSubTextureMesh->texture.singleChannel);
        }
        // Render mesh
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, NULL);
    };
}

#endif