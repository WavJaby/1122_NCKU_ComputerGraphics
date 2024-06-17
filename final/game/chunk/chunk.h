#ifndef __CHUNK_H__
#define __CHUNK_H__

#include "chunk_settings.h"
#include "chunk_sub.h"

typedef struct ChunkCoord {
    int x, z;
} ChunkCoord;

typedef struct Chunk {
    int chunkX, chunkZ;
    struct ChunkSub* chunkSub[TOTAL_CHUNK_SUB_HEIGHT];

    struct Chunk* front;
    struct Chunk* back;
    struct Chunk* left;
    struct Chunk* right;
} Chunk;
Chunk* chunk_new(Map* chunks, int chunkX, int chunkZ) {
    Chunk* chunk = calloc(1, sizeof(Chunk));
    chunk->chunkX = chunkX;
    chunk->chunkZ = chunkZ;

    // Register chunk neighbor
    ChunkCoord coordNext = {chunkX - 1, chunkZ};
    Chunk* left = (Chunk*)map_get(chunks, &coordNext);
    if (left) {
        left->right = chunk;
        chunk->left = left;
    }
    coordNext.x = chunkX + 1;
    Chunk* right = (Chunk*)map_get(chunks, &coordNext);
    if (right) {
        right->left = chunk;
        chunk->right = right;
    }
    coordNext.x = chunkX;
    coordNext.z = chunkZ + 1;
    Chunk* front = (Chunk*)map_get(chunks, &coordNext);
    if (front) {
        front->back = chunk;
        chunk->front = front;
    }
    coordNext.z = chunkZ - 1;
    Chunk* back = (Chunk*)map_get(chunks, &coordNext);
    if (back) {
        back->front = chunk;
        chunk->back = back;
    }

    return chunk;
}

#define chunk_getChunkSubIndexByBlockY(y) (y >> CHUNK_SUB_Y_SIZE_SHIFT) - NEGTIVE_INDEX_HEIGHT
struct ChunkSub* chunk_getChunkSubByBlockY(Chunk* chunk, int y) {
    return chunk->chunkSub[chunk_getChunkSubIndexByBlockY(y)];
}

#endif