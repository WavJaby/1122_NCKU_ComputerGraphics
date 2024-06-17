#ifndef __BLOCK_H__
#define __BLOCK_H__

typedef struct BlockModelElementFaceData {
    Texture texture;
    // [x,y] * 4
    float uv[8];
} BlockModelElementFaceData;
BlockModelElementFaceData* block_newBlockModelElementFaceData(Texture texture, float uv[8]) {
    BlockModelElementFaceData* obj = malloc(sizeof(BlockModelElementFaceData));
    obj->texture = texture;
    memcpy(obj->uv, uv, sizeof(obj->uv));
    return obj;
}

/**
 * @brief Fixed size
 *
 */
typedef struct BlockModelElement {
    vec3 from, to;
    // x,-x, y,-y, z,-z
    BlockModelElementFaceData* faces[6];
    uint8_t cullFaces[6];  // cull face index
    uint32_t faceCount;
} BlockModelElement;

typedef struct BlockModel {
    BlockModelElement* elements;
    int elementsCount;
    bool fullBlock;
} BlockModel;
BlockModel* block_newBlockModel(BlockModelElement* elements, int elementsCount, bool fullBlock) {
    BlockModel* obj = malloc(sizeof(BlockModel));
    obj->elements = malloc(elementsCount * sizeof(BlockModelElement));
    memcpy(obj->elements, elements, elementsCount * sizeof(BlockModelElement));
    obj->elementsCount = elementsCount;
    obj->fullBlock = fullBlock;
    return obj;
}
void block_freeBlockModel(BlockModel* blockModel) {
    for (size_t i = 0; i < blockModel->elementsCount; i++) {
        for (size_t j = 0; j < 6; j++) {
            free(blockModel->elements[i].faces[j]);
        }
    }
    free(blockModel->elements);
    free(blockModel);
}

typedef struct BlockMesh {
    uint8_t x, y, z;
} BlockMesh;

typedef struct Block {
    // ChunkSub* chunkSub;
    uint8_t xInChunk, yInChunk, zInChunk;
    BlockModel* model;
} Block;

#endif