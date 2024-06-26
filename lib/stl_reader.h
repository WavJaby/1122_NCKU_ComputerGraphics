#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gl_vector.h"

typedef struct _STriangle {
    float normal[3], a[3], b[3], c[3];
} STriangle;

typedef struct _STrianglesInfo {
    Vector3f boundMax, boundMin;
    Vector3f center;
    STriangle* triangles;
    uint32_t trianglesCount;
    float maxSize;
} STrianglesInfo;

void calculateTriangleInfo(STriangle* triangles, uint32_t trianglesCount, STrianglesInfo* info) {
    info->trianglesCount = trianglesCount;
    info->triangles = triangles;
    if (trianglesCount == 0) {
        vec3fZero(info->boundMin);
        vec3fZero(info->boundMax);
        vec3fZero(info->center);
        return;
    }
    float min[3], max[3];
    vec3fZero(info->boundMin);
    vec3fZero(info->boundMax);

    for (uint32_t i = 0; i < trianglesCount; i++) {
        if (i == 0) {
            for (uint8_t axis = 0; axis < 3; axis++) {
                min[axis] = max[axis] = triangles[i].a[axis];
            }
        }

        for (uint8_t axis = 0; axis < 3; axis++) {
            if (triangles[i].a[axis] < min[axis])
                min[axis] = triangles[i].a[axis];
            else if (triangles[i].a[axis] > max[axis])
                max[axis] = triangles[i].a[axis];
            if (triangles[i].b[axis] < min[axis])
                min[axis] = triangles[i].b[axis];
            else if (triangles[i].b[axis] > max[axis])
                max[axis] = triangles[i].b[axis];
            if (triangles[i].c[axis] < min[axis])
                min[axis] = triangles[i].c[axis];
            else if (triangles[i].c[axis] > max[axis])
                max[axis] = triangles[i].c[axis];
        }
    }
    vx(info->center) = (min[0] + max[0]) / 2.0f;
    vy(info->center) = (min[1] + max[1]) / 2.0f;
    vz(info->center) = (min[2] + max[2]) / 2.0f;

    vx(info->boundMin) = min[0];
    vy(info->boundMin) = min[1];
    vz(info->boundMin) = min[2];

    vx(info->boundMax) = max[0];
    vy(info->boundMax) = max[1];
    vz(info->boundMax) = max[2];

    float wy = max[1] - min[1], wz = max[2] - min[2];
    info->maxSize = max[0] - min[0];
    if (wy > info->maxSize)
        info->maxSize = wy;
    if (wz > info->maxSize)
        info->maxSize = wz;
}

const char* solid = "solid ";
void loadStlASCII(FILE* file, STrianglesInfo* trianglesInfo) {
    const char* endsolid = "endsolid";
    const char endsolidLen = strlen(endsolid);
    const char* facet = "facet";
    const int facetLen = strlen(facet);
    const char* endfacet = "endfacet";
    const int endfacetLen = strlen(endfacet);
    const char* normal = "normal";
    const int normalLen = strlen(normal);
    const char* outer = "outer";
    const int outerLen = strlen(outer);
    const char* loop = "loop";
    const int loopLen = strlen(loop);
    const char* endloop = "endloop";
    const int endloopLen = strlen(endloop);
    const char* vertex = "vertex";
    const int vertexLen = strlen(vertex);

    char str[256];
    if (!fgets(str, sizeof(str), file) || strncmp(str, solid, strlen(solid)) != 0) {
        fprintf(stderr, "Invalid STL file\n");
        fclose(file);
        return;
    }

    STriangle* triList = NULL;
    uint32_t listSize = 0;
    uint32_t capacity = 10;  // Initial capacity
    triList = (STriangle*)malloc(sizeof(STriangle) * capacity);

    while (!feof(file)) {
        STriangle* tri = triList + listSize;
        if (fscanf(file, "%s", str) != 1) {
            fprintf(stderr, "Invalid STL file\n");
            goto ERROR_HANDLE;
        }

        if (strncmp(str, endsolid, endsolidLen) == 0)
            break;

        if (strncmp(str, facet, facetLen) != 0) {
            fprintf(stderr, "Invalid STL file at facet\n");
            goto ERROR_HANDLE;
        }

        // Read normal
        if (fscanf(file, "%s", str) != 1 || strncmp(str, normal, normalLen) != 0) {
            fprintf(stderr, "Invalid STL file at normal\n");
            goto ERROR_HANDLE;
        }
        if (fscanf(file, "%f %f %f", &tri->normal[0], &tri->normal[1], &tri->normal[2]) != 3) {
            fprintf(stderr, "Failed to read normal vector\n");
            goto ERROR_HANDLE;
        }

        // Read outer
        if (fscanf(file, "%s", str) != 1 || strncmp(str, outer, outerLen) != 0) {
            fprintf(stderr, "Invalid STL file at outer\n");
            goto ERROR_HANDLE;
        }
        if (fscanf(file, "%s", str) != 1 || strncmp(str, loop, loopLen) != 0) {
            fprintf(stderr, "Invalid STL file at outer type: '%s'\n", str);
            goto ERROR_HANDLE;
        }
        for (int i = 0; i < 3; i++) {
            if (fscanf(file, "%s", str) != 1 || strncmp(str, vertex, vertexLen) != 0) {
                fprintf(stderr, "Invalid STL file at vertex\n");
                goto ERROR_HANDLE;
            }

            float* v = (i == 0) ? tri->a : ((i == 1) ? tri->b : tri->c);
            if (fscanf(file, "%f %f %f", &v[0], &v[1], &v[2]) != 3) {
                fprintf(stderr, "Failed to read vertex\n");
                goto ERROR_HANDLE;
            }
        }
        if (fscanf(file, "%s", str) != 1 || strncmp(str, endloop, endloopLen) != 0) {
            fprintf(stderr, "Invalid STL file at outer type close\n");
            goto ERROR_HANDLE;
        }
        if (fscanf(file, "%s", str) != 1 || strncmp(str, endfacet, endfacetLen) != 0) {
            fprintf(stderr, "Invalid STL file at facet close\n");
            goto ERROR_HANDLE;
        }

        // re-compute normal
        // float vb[3] = {tri.b[0] - tri.a[0], tri.b[1] - tri.a[1], tri.b[2] - tri.a[2]};
        // float vc[3] = {tri.c[0] - tri.a[0], tri.c[1] - tri.a[1], tri.c[2] - tri.a[2]};
        // float normal[3] = {vb[1] * vc[2] - vb[2] * vc[1], vb[2] * vc[0] - vb[0] * vc[2], vb[0] * vc[1] - vb[1] * vc[0]};
        // float len = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
        // if (len > 0) {
        //     normal[0] /= len;
        //     normal[1] /= len;
        //     normal[2] /= len;
        // }
        // tri.n[0] = normal[0];
        // tri.n[1] = normal[1];
        // tri.n[2] = normal[2];

        listSize++;
        if (listSize >= capacity) {
            capacity *= 2;
            triList = (STriangle*)realloc(triList, sizeof(STriangle) * capacity);
        }
    }

    fclose(file);
    calculateTriangleInfo(triList, listSize, trianglesInfo);
    return;
ERROR_HANDLE:
    fclose(file);
    calculateTriangleInfo(triList, listSize, trianglesInfo);
    free(triList);
}

void loadStlBinary(FILE* file, STrianglesInfo* trianglesInfo) {
    uint8_t tmpBuff[80];
    size_t len = fread(tmpBuff, 1, sizeof(tmpBuff), file);
    if (len != sizeof(tmpBuff)) {
        fprintf(stderr, "Invalid STL file: Bad header\n");
        goto ERROR_HANDLE;
    }

    // Read triangle count
    uint32_t listSize = 0;
    fread(&listSize, 4, 1, file);
    if (listSize < 0) {
        fprintf(stderr, "Invalid STL file: Triangle count %d\n", listSize);
        goto ERROR_HANDLE;
    }

    // Allocate array
    STriangle* triList = (STriangle*)malloc(sizeof(STriangle) * listSize);

    for (size_t i = 0; i < listSize; i++) {
        len = fread(tmpBuff, 1, 50, file);
        if (len != 50) {
            fprintf(stderr, "Invalid STL file: Bad triangle data length\n");
            goto ERROR_HANDLE;
        }
        triList[i].normal[0] = *(float*)(tmpBuff);
        triList[i].normal[1] = *(float*)(tmpBuff + 4);
        triList[i].normal[2] = *(float*)(tmpBuff + 8);

        triList[i].a[0] = *(float*)(tmpBuff + 12);
        triList[i].a[1] = *(float*)(tmpBuff + 16);
        triList[i].a[2] = *(float*)(tmpBuff + 20);

        triList[i].b[0] = *(float*)(tmpBuff + 24);
        triList[i].b[1] = *(float*)(tmpBuff + 28);
        triList[i].b[2] = *(float*)(tmpBuff + 32);

        triList[i].c[0] = *(float*)(tmpBuff + 36);
        triList[i].c[1] = *(float*)(tmpBuff + 40);
        triList[i].c[2] = *(float*)(tmpBuff + 44);

        // uint16_t attrCount = *(uint16_t*)(tmpBuff + 48);
    }

    fclose(file);
    calculateTriangleInfo(triList, listSize, trianglesInfo);
    return;
ERROR_HANDLE:
    fclose(file);
    calculateTriangleInfo(triList, listSize, trianglesInfo);
    free(triList);
}

void loadStl(const char* fileName, STrianglesInfo* trianglesInfo) {
    if (!fileName) {
        fprintf(stderr, "Invalid file path\n");
        return;
    }

    FILE* file = fopen(fileName, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file '%s'\n", fileName);
        return;
    }

    char str[256];
    if (!fgets(str, sizeof(str), file)) {
        fprintf(stderr, "Failed to read file '%s'\n", fileName);
        return;
    }
    rewind(file);

    if (strncmp(str, solid, strlen(solid)) == 0) {
        loadStlASCII(file, trianglesInfo);
    } else {
        loadStlBinary(file, trianglesInfo);
    }
}