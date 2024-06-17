#include "chunk_settings.h"

typedef struct Chunk {
    ChunkSub* chunkSub[TOTAL_CHUNK_SUB_HEIGHT];
} Chunk;