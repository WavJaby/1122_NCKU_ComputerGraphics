#include <pthread.h>

typedef struct GameManager {
    BlockModel* grassBlockModel;
    BlockModel* dirtBlockModel;
    BlockModel* stoneBlockModel;
    BlockModel* sandBlockModel;
    BlockModel* oakLogBlockModel;
    BlockModel* shortGrassBlockModel;
} GameManager;
GameManager gameManager = {};

bool chunkCoordEquals(void* a, void* b) {
    return ((ChunkCoord*)a)->x == ((ChunkCoord*)b)->x &&
           ((ChunkCoord*)a)->z == ((ChunkCoord*)b)->z;
}
uint32_t chunkCoordHash(void* a) {
    return ((ChunkCoord*)a)->x + ((ChunkCoord*)a)->z << 16;
}
// Map<ChunkCoord*, Chunk*>
Map chunks = map_create(chunkCoordEquals,
                        chunkCoordHash,
                        NULL,
                        WJCL_HASH_MAP_FREE_KEY);

void* generateWorld(void* data);

int viewDistance = 20;
int cx, cz;
int state = 0;
pthread_t worldLoadThread;

void gameManagerOnStart() {
    Texture dirt = texture_fromFile("../minecraft/textures/block/dirt.png", (vec4){1, 1, 1, 1});
    Texture grass_block_top = texture_fromFile("../minecraft/textures/block/grass_block_top.png", (vec4){99 / 255.f, 185 / 255.f, 39 / 255.f, 1});
    gameManager.grassBlockModel = block_newBlockModel(
        (BlockModelElement[1]){
            {
                .from = {0, 0, 0},
                .to = {16, 16, 16},
                .cullFaces = {0, 1, 2, 3, 4, 5},
                .faces = {
                    block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(grass_block_top, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                },
                .faceCount = 6,
            }},
        1, true);

    gameManager.dirtBlockModel = block_newBlockModel(
        (BlockModelElement[1]){{
            .from = {0, 0, 0},
            .to = {16, 16, 16},
            .cullFaces = {0, 1, 2, 3, 4, 5},
            .faces = {
                block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(dirt, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
            },
            .faceCount = 6,
        }},
        1, true);

    Texture stone = texture_fromFile("../minecraft/textures/block/stone.png", (vec4){1, 1, 1, 1});
    gameManager.stoneBlockModel = block_newBlockModel(
        (BlockModelElement[1]){{
            .from = {0, 0, 0},
            .to = {16, 16, 16},
            .cullFaces = {0, 1, 2, 3, 4, 5},
            .faces = {
                block_newBlockModelElementFaceData(stone, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(stone, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(stone, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(stone, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(stone, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(stone, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
            },
            .faceCount = 6,
        }},
        1, true);

    Texture sand = texture_fromFile("../minecraft/textures/block/sand.png", (vec4){1, 1, 1, 1});
    gameManager.sandBlockModel = block_newBlockModel(
        (BlockModelElement[1]){{
            .from = {0, 0, 0},
            .to = {16, 16, 16},
            .cullFaces = {0, 1, 2, 3, 4, 5},
            .faces = {
                block_newBlockModelElementFaceData(sand, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(sand, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(sand, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(sand, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(sand, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(sand, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
            },
            .faceCount = 6,
        }},
        1, true);

    Texture oak_log = texture_fromFile("../minecraft/textures/block/oak_log.png", (vec4){1, 1, 1, 1});
    Texture oak_log_top = texture_fromFile("../minecraft/textures/block/oak_log_top.png", (vec4){1, 1, 1, 1});
    gameManager.oakLogBlockModel = block_newBlockModel(
        (BlockModelElement[1]){{
            .from = {0, 0, 0},
            .to = {16, 16, 16},
            .cullFaces = {0, 1, 2, 3, 4, 5},
            .faces = {
                block_newBlockModelElementFaceData(oak_log, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(oak_log, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(oak_log_top, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(oak_log_top, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(oak_log, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                block_newBlockModelElementFaceData(oak_log, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
            },
            .faceCount = 6,
        }},
        1, true);

    Texture short_grass = texture_fromFile("../minecraft/textures/block/short_grass.png", (vec4){99 / 255.f, 185 / 255.f, 39 / 255.f, 1});
    gameManager.shortGrassBlockModel = block_newBlockModel(
        (BlockModelElement[2]){
            {
                .from = {0.8, 0, 8},
                .to = {15.2, 16, 8},
                .cullFaces = {-1, -1, -1, -1, -1, -1},
                .faces = {
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    block_newBlockModelElementFaceData(short_grass, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(short_grass, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                },
                .faceCount = 2,
            },
            {
                .from = {8, 0, 0.8},
                .to = {8, 16, 15.2},
                .cullFaces = {-1, -1, -1, -1, -1, -1},
                .faces = {
                    block_newBlockModelElementFaceData(short_grass, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    block_newBlockModelElementFaceData(short_grass, (float[8]){0, 0, 0, 1, 1, 1, 1, 0}),
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                },
                .faceCount = 2,
            }},
        2, false);
}

void renderChunks(bool renderShadow) {
    mat4x4 model;
    // Render chunks
    map_entries(&chunks, entries) {
        Chunk* chunk = (Chunk*)entries->value;
        for (size_t i = 0; i < TOTAL_CHUNK_SUB_HEIGHT; i++) {
            ChunkSub* chunkSub = chunk->chunkSub[i];
            if (!chunkSub || chunkSub->state != CHUNK_SUB_MESH_CREATED) continue;

            // Render ChunkSub
            ChunkCoord* chunkCoord = (ChunkCoord*)entries->key;
            mat4x4_translate_create(model, chunkCoord->x << CHUNK_SIZE_X_SHIFT, (chunkSub->subIndex + NEGTIVE_INDEX_HEIGHT) << CHUNK_SUB_Y_SIZE_SHIFT, chunkCoord->z << CHUNK_SIZE_Z_SHIFT);
            if (renderShadow) glUniformMatrix4fv(depthShader.uModel, 1, GL_FALSE, (float*)model);
            else glUniformMatrix4fv(objectShader.uModel, 1, GL_FALSE, (float*)model);

            chunkSub_render(chunkSub, renderShadow);
        }
    }
}

void gameManagerOnRender() {
    if (state == 0) {
        state = 1;
        pthread_create(&worldLoadThread, NULL, generateWorld, "GenerateWorld");

        cx = -viewDistance;
        cz = -viewDistance;
    } else if (state == 2) {
        // Init chunk mesh
        for (size_t i = 0; i < 5; i++) {
            if (cx < viewDistance) {
                if (cz < viewDistance) {
                    ChunkCoord coordNext = {cx, cz};
                    Chunk* chunk = (Chunk*)map_get(&chunks, &coordNext);
                    // Check if chunk is loaded
                    if (chunk) {
                        // Check each ChunkSub
                        int loadCount = 0;
                        for (size_t i = 0; i < TOTAL_CHUNK_SUB_HEIGHT; i++) {
                            ChunkSub* chunkSub = chunk->chunkSub[i];
                            if (!chunkSub || !chunkSub_isReadyInitMesh(chunkSub)) continue;
                            chunkSub_initMeshVertices(chunkSub);
                            chunkSub_initMesh(chunkSub);
                            chunkSub->state = CHUNK_SUB_MESH_CREATED;
                            loadCount++;
                        }

                        // printf("Loading Chunk: %d, %d, %d\n", cx, cz, loadCount);
                    }
                    cz++;
                } else {
                    cz = -viewDistance;
                    cx++;
                }
            }
        }
        renderChunks(false);
    }
}

void gameManagerOnRenderShadow() {
    if (state == 2) {
        renderChunks(true);
    }
}

void gameManagerOnRenderUI() {
    if (state == 1) {
        ui_textDrawStringCenter("Generating World...", windowWidth / 2, windowHeight / 2, windowWidth * 0.05f);
    }
}

float terrain(int x, int y, float size) {
    float val = 0;

    float freq = 1;
    float amp = 1;

    // Sampling
    for (int i = 0; i < 3; i++) {
        val += perlin(x * freq / size, y * freq / size) * amp;

        freq *= 2;
        amp /= 2;
    }

    // Contrast
    val *= 1.4;

    // Clipping
    if (val > 1.0f)
        val = 1.0f;
    else if (val < -1.0f)
        val = -1.0f;

    // Normalize
    val = (val + 1) * 0.5f;
    return val;
}

void* generateWorld(void* data) {
    int hViewDistance = viewDistance + 1;
    for (int cx = -hViewDistance; cx < hViewDistance; cx++) {
        for (int cz = -hViewDistance; cz < hViewDistance; cz++) {
            if (cx * cx + cz * cz > hViewDistance * hViewDistance) continue;

            Chunk* chunk = chunk_new(&chunks, cx, cz);

            // Set chunk sub block ready
            for (int cy = -1; cy < 4; cy++) {
                ChunkSub* chunkSub = chunk->chunkSub[cy - NEGTIVE_INDEX_HEIGHT] = chunkSub_newWithIndex(chunk, cy - NEGTIVE_INDEX_HEIGHT);
                chunkSub->state = CHUNK_SUB_BLOCK_LOADED;
            }

            // Create chunk blocks
            int xOff = cx * CHUNK_SIZE_X + 65535, zOff = cz * CHUNK_SIZE_Z + 65535;
            for (uint8_t x = 0; x < 16; x++) {
                for (uint8_t z = 0; z < 16; z++) {
                    float h = terrain(xOff + x, zOff + z, 1000);
                    h = h * 10;
                    h *= terrain(xOff + x, zOff + z, 100);
                    h = h * 10 + 1;

                    // Create y blocks
                    for (int y = -1; y <= (int)h; y++) {
                        Block* block = (Block*)malloc(sizeof(Block));
                        block->xInChunk = x;
                        block->yInChunk = y & CHUNK_SUB_Y_SIZE_MASK;
                        block->zInChunk = z;

                        if (y == (int)h && y < 4)
                            block->model = gameManager.sandBlockModel;
                        else if (y == (int)h)
                            block->model = gameManager.grassBlockModel;
                        else if (y + 5 > h)
                            block->model = gameManager.dirtBlockModel;
                        else
                            block->model = gameManager.stoneBlockModel;

                        // Get ChunkSub
                        ChunkSub* chunkSub = chunk->chunkSub[chunk_getChunkSubIndexByBlockY(y)];

                        chunkSub_setBlock(chunkSub, block);
                    }
                    // Plant grass
                    float grassProp = terrain(xOff + x, zOff + z, 1.1f);
                    if (grassProp < 0.2) {
                        int y = (int)h + 1;
                        if (y > 4) {
                            Block* block = (Block*)malloc(sizeof(Block));
                            block->xInChunk = x;
                            block->yInChunk = y & CHUNK_SUB_Y_SIZE_MASK;
                            block->zInChunk = z;
                            block->model = gameManager.shortGrassBlockModel;

                            ChunkSub* chunkSub = chunk->chunkSub[chunk_getChunkSubIndexByBlockY(y)];
                            chunkSub_setBlock(chunkSub, block);
                        }
                    }
                }
            }
        }
    }
    state = 2;
    pthread_exit(NULL);
}