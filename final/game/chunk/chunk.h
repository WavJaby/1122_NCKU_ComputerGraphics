#include "chunk_settings.h"

typedef struct Chunk {
    ChunkSub* chunk[TOTAL_CHUNK_SUB_HEIGHT];
} Chunk;