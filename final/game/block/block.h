#ifndef __BLOCK_H__
#define __BLOCK_H__

typedef struct BlockModelElementFaceData {
    MeshTexture texture;
    // [x,y] * 4
    float uv[8];
} BlockModelElementFaceData;

typedef struct BlockModelElement {
    vec3 from, to;
    // x,-x, y,-y, z,-z
    BlockModelElementFaceData* faces[6];
    uint8_t cullFaces[6];  // cull face index
    uint32_t faceCount;
} BlockModelElement;

typedef struct BlockModel {
    BlockModelElement* elements;
    int elementsLen;
    bool fullBlock;
} BlockModel;

typedef struct BlockMesh {
    uint8_t x, y, z;
} BlockMesh;

typedef struct Block {
    // ChunkSub* chunkSub;
    uint8_t xInChunk, yInChunk, zInChunk;
    BlockModel* model;
} Block;

#endif