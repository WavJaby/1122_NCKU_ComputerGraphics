typedef struct GameManager {
    BlockModel* grassBlockModel;
} GameManager;
GameManager gameManager = {};

typedef struct ChunkCoord {
    int x, z;
} ChunkCoord;

bool chunkCoordEquals(void* a, void* b) {
    return ((ChunkCoord*)a)->x == ((ChunkCoord*)b)->x &&
           ((ChunkCoord*)a)->z == ((ChunkCoord*)b)->z;
}
uint32_t chunkCoordHash(void* a) {
    return ((ChunkCoord*)a)->x + ((ChunkCoord*)a)->z << 16;
}
// Map<ChunkCoord*, ChunkSub*>
Map chunks = map_create(chunkCoordEquals,
                        chunkCoordHash,
                        NULL,
                        WJCL_HASH_MAP_FREE_KEY);

void generateWorld();

uint64_t framCount = 1;
int cx = -20;
int state = 0;

void gameManagerOnStart() {
    Texture dirt = texture_fromFile("../minecraft/textures/block/dirt.png", (vec4){1, 1, 1, 1});

    Texture grass_block_top = texture_fromFile("../minecraft/textures/block/grass_block_top.png",
                                               (vec4){99 / 255.f, 185 / 255.f, 39 / 255.f, 1});

    Texture oak_log = texture_fromFile("../minecraft/textures/block/oak_log.png", (vec4){1, 1, 1, 1});

    Texture oak_log_top = texture_fromFile("../minecraft/textures/block/oak_log_top.png", (vec4){1, 1, 1, 1});

    BlockModelElement element[1] = {
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
        }};
    gameManager.grassBlockModel = block_newBlockModel(element, 1, true);
}

void gameManagerOnRender() {
    if (framCount++ % 30 == 0)
        if (state == 0) {
            generateWorld();
            state++;
        } else {
            // Init chunk mesh
            if (cx < 20) {
                for (int cz = -20; cz < 20; cz++) {
                    ChunkCoord coordNext = {cx, cz};
                    ChunkSub* chunkSub = (ChunkSub*)map_get(&chunks, &coordNext);
                    chunkSub_initMeshVertices(chunkSub);
                    chunkSub_initMesh(chunkSub);
                }
                cx++;
            }
        }

    mat4x4 model;
    // Render chunks
    map_entries(&chunks, entries) {
        ChunkSub* chunkSub = (ChunkSub*)entries->value;
        ChunkCoord* chunkCoord = (ChunkCoord*)entries->key;
        mat4x4_translate_create(model, chunkCoord->x * CHUNK_SIZE_X, 0, chunkCoord->z * CHUNK_SIZE_Z);
        glUniformMatrix4fv(objectShader.uModel, 1, GL_FALSE, (float*)model);

        chunkSub_render(chunkSub);
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

void generateWorld() {
    for (int cx = -21; cx < 21; cx++) {
        for (int cz = -21; cz < 21; cz++) {
            ChunkCoord* coord = malloc(sizeof(ChunkCoord));
            coord->x = cx;
            coord->z = cz;
            ChunkSub* chunkSub = chunkSub_new();
            chunkSub->chunkX = cx;
            chunkSub->chunkZ = cz;
            chunkSub->chunkY = 0;
            map_putpp(&chunks, coord, chunkSub);

            ChunkCoord coordNext = {cx - 1, cz};
            ChunkSub* left = (ChunkSub*)map_get(&chunks, &coordNext);
            if (left) {
                left->right = chunkSub;
                chunkSub->left = left;
            }
            coordNext.x = cx + 1;
            ChunkSub* right = (ChunkSub*)map_get(&chunks, &coordNext);
            if (right) {
                right->left = chunkSub;
                chunkSub->right = right;
            }
            coordNext.x = cx;
            coordNext.z = cz + 1;
            ChunkSub* front = (ChunkSub*)map_get(&chunks, &coordNext);
            if (front) {
                front->back = chunkSub;
                chunkSub->front = front;
            }
            coordNext.z = cz - 1;
            ChunkSub* back = (ChunkSub*)map_get(&chunks, &coordNext);
            if (back) {
                back->front = chunkSub;
                chunkSub->back = back;
            }

            // Create chunk blocks
            int xOff = cx * CHUNK_SIZE_X + 65535, zOff = cz * CHUNK_SIZE_Z + 65535;
            for (uint8_t x = 0; x < 16; x++) {
                for (uint8_t z = 0; z < 16; z++) {
                    int h = terrain(xOff + x, zOff + z, 100) * 16 + 1;

                    for (uint8_t y = 0; y < h; y++) {
                        Block* block = (Block*)malloc(sizeof(Block));
                        block->xInChunk = x;
                        block->yInChunk = y;
                        block->zInChunk = z;
                        block->model = gameManager.grassBlockModel;
                        chunkSub_setBlock(chunkSub, block);
                    }
                }
            }
        }
    }
}