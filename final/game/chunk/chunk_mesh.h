
typedef struct BlockModelElementFaceData {
    int testureId;
    // [x,y] * 4
    float uv[8];
} BlockModelElementFaceData;

typedef struct BlockModelElement {
    vec3 from, to;
    // x,-x, y,-y, z,-z
    BlockModelElementFaceData* faces[6];
    uint8_t cullFaces[6];  // cull face index
} BlockModelElement;

typedef struct BlockModel {
    BlockModelElement* elements;
    int elementsLen;
} BlockModel;

typedef struct BlockMesh {
    int x, y, z;
} BlockMesh;

typedef struct ChunkSubMesh {
    // x,y,z, u,v, x,y,z
    float* vertices_uv_normal;
    uint32_t* indices;
    // 1 face contains 2 triangle
    int faceCount;
} ChunkSubMesh;

#define setVertexData(vertices, index, \
                      u, v,            \
                      x, y, z,         \
                      nx, ny, nz)      \
    vertices[index] = x;               \
    vertices[index + 1] = y;           \
    vertices[index + 2] = z;           \
    vertices[index + 3] = u;           \
    vertices[index + 4] = v;           \
    vertices[index + 5] = nx;          \
    vertices[index + 6] = ny;          \
    vertices[index + 7] = nz;          \
    index += 8;

/**
 * @brief
 *
 * @param chunkSubMesh target mesh to render
 * @param blockMesh rendered block model infomation
 * @param elementIndex element index in model
 * @param face 6 bytes, -z,z,-y,y,-x,x
 * @param element block model element to render
 * @param rotateIndex face index after rotation
 * @param rotX block rotation x
 * @param rotY block rotation y
 * @param uvLock local texture rotation on +y, -y face
 */
void AddFace(ChunkSubMesh* chunkSubMesh, BlockMesh* blockMesh, int elementIndex,
             uint8_t face, BlockModelElement* element,
             uint8_t rotateIndex[6],
             int rotX, int rotY, bool uvLock) {
    uint8_t newFaceCount = (uint8_t)((face & 0b1) + (face >> 1 & 0b1) + (face >> 2 & 0b1) + (face >> 3 & 0b1) + (face >> 4 & 0b1) + (face >> 5 & 0b1));
    if (newFaceCount == 0) return;

    // if ((_faceCount + newFaceCount) * 4 > _vertices.Length) {
    //     int size = (int)(_faceCount * 1.5f) * 4;
    //     Array.Resize(ref _vertices, size + 1);
    //     Array.Resize(ref _uv, size + 1);
    // }

    // if (_faceCount + newFaceCount > _faceIndex.Length)
    //     Array.Resize(ref _faceIndex, (int)(_faceIndex.Length * 1.5f));
    // if ((_faceCount + newFaceCount) * 6 > _indices.Length)
    //     Array.Resize(ref _indices, (int)(_faceCount * 1.5f) * 6);

    // int triCnt = _faceCount * 6;

    // 到目前所有的頂點
    // int cnt = _faceCount * 4 + 1;

    uint32_t* _indices = chunkSubMesh->indices;
    float* _vertices_uv_normal = chunkSubMesh->vertices_uv_normal;

    // total indices count
    int indicesLen = chunkSubMesh->faceCount * 6;
    int vertexIndex = chunkSubMesh->faceCount * 4;
    // total vertices float array count
    int verticesSize = chunkSubMesh->faceCount * 8 * 4;

    float _modelScale = 1.f / 16;
    float from[3] = {element->from[0] * _modelScale, element->from[1] * _modelScale, 1 - element->to[2] * _modelScale};
    float to__[3] = {element->to[0] * _modelScale, element->to[1] * _modelScale, 1 - element->from[2] * _modelScale};

    // Vector3 center;
    // Vector3 blockCenter;
    // Quaternion rotation;
    // Quaternion blockRotation;
    // bool rotate, blockRotate = rotX != 0 || rotY != 0;
    // if (blockRotate) {
    //     blockCenter = new Vector3(
    //         _defaultBlockCenter.x + blockMesh->x,
    //         _defaultBlockCenter.y + blockMesh->y,
    //         _defaultBlockCenter.z + blockMesh->z);
    //     blockRotation = Quaternion.Euler(rotX, rotY, 0);
    // } else {
    //     blockCenter = default;
    //     blockRotation = default;
    // }
    // if (element->Rotation != null) {
    //     center = new Vector3(
    //         blockMesh->x + element->Rotation.Origin[0] / _textureSize,
    //         blockMesh->y + element->Rotation.Origin[1] / _textureSize,
    //         blockMesh->z + 1f - element->Rotation.Origin[2] / _textureSize);
    //     rotation = Quaternion.Euler(-element->Rotation.Angle[0], element->Rotation.Angle[1], element->Rotation.Angle[2]);
    //     rotate = element->Rotation.Angle[0] != 0 || element->Rotation.Angle[1] != 0 || element->Rotation.Angle[2] != 0;
    //     // Debug.Log($"{rotateX},{rotateY}, {rotate}");
    // } else {
    //     center = default;
    //     rotation = default;
    //     rotate = false;
    // }

    // x
    if ((face & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[0];
        if (!faceData) {
            fprintf(stderr, "FaceData +x is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[0]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float x = blockMesh->x + to__[0];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], x, blockMesh->y + from[1], blockMesh->z + from[2], 1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], x, blockMesh->y + to__[1], blockMesh->z + from[2], 1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], x, blockMesh->y + to__[1], blockMesh->z + to__[2], 1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], x, blockMesh->y + from[1], blockMesh->z + to__[2], 1, 0, 0);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // -x
    if ((face >> 1 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[1];
        if (!faceData) {
            fprintf(stderr, "FaceData -x is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[1]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float x = blockMesh->x + from[0];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], x, blockMesh->y + from[1], blockMesh->z + to__[2], -1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], x, blockMesh->y + to__[1], blockMesh->z + to__[2], -1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], x, blockMesh->y + to__[1], blockMesh->z + from[2], -1, 0, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], x, blockMesh->y + from[1], blockMesh->z + from[2], -1, 0, 0);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // y
    if ((face >> 2 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[2];
        if (!faceData) {
            fprintf(stderr, "FaceData +y is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[2]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float y = blockMesh->y + to__[1];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + from[0], y, blockMesh->z + from[2], 0, 1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + from[0], y, blockMesh->z + to__[2], 0, 1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + to__[0], y, blockMesh->z + to__[2], 0, 1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + to__[0], y, blockMesh->z + from[2], 0, 1, 0);
        // if (uvLock)
        //     RotateTexture(cnt, -rotY);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // -y
    if ((face >> 3 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[3];
        if (!faceData) {
            fprintf(stderr, "FaceData -y is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[3]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float y = blockMesh->y + from[1];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + to__[0], y, blockMesh->z + from[2], 0, -1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + to__[0], y, blockMesh->z + to__[2], 0, -1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + from[0], y, blockMesh->z + to__[2], 0, -1, 0);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + from[0], y, blockMesh->z + from[2], 0, -1, 0);
        // if (uvLock)
        //     RotateTexture(cnt, -rotY);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // z
    if ((face >> 4 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[4];
        if (!faceData) {
            fprintf(stderr, "FaceData +z is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[4]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex + 3;
        vertexIndex += 4;
        float z = blockMesh->z + to__[2];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + to__[0], blockMesh->y + from[1], z, 0, 0, 1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + to__[0], blockMesh->y + to__[1], z, 0, 0, 1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + from[0], blockMesh->y + to__[1], z, 0, 0, 1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + from[0], blockMesh->y + from[1], z, 0, 0, 1);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }

    // -z
    if ((face >> 5 & 0b1) == 0b1) {
        BlockModelElementFaceData* faceData = element->faces[5];
        if (!faceData) {
            fprintf(stderr, "FaceData -z is null\n");
            return;
        }
        // blockMesh->index[elementIndex][rotateIndex[5]] = _faceIndex[_faceCount] =
        //     new FaceIndex(_faceCount++);
        chunkSubMesh->faceCount++;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 1;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen++] = vertexIndex;
        _indices[indicesLen++] = vertexIndex + 2;
        _indices[indicesLen] = vertexIndex + 3;
        float z = blockMesh->z + from[2];
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[0], faceData->uv[1], blockMesh->x + from[0], blockMesh->y + from[1], z, 0, 0, -1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[2], faceData->uv[3], blockMesh->x + from[0], blockMesh->y + to__[1], z, 0, 0, -1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[4], faceData->uv[5], blockMesh->x + to__[0], blockMesh->y + to__[1], z, 0, 0, -1);
        setVertexData(_vertices_uv_normal, verticesSize, faceData->uv[6], faceData->uv[7], blockMesh->x + to__[0], blockMesh->y + from[1], z, 0, 0, -1);
        // if (rotate)
        //     RotateFace(cnt, center, rotation, element->Rotation.Rescale);
        // if (blockRotate)
        //     RotateFace(cnt, blockCenter, blockRotation);
    }
}
