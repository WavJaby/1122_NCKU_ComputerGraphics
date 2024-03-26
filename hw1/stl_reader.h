#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/gl_vector.h"

typedef struct _STriangle {
    float normal[3], a[3], b[3], c[3];
} STriangle;

typedef struct _STrianglesInfo {
    GLVector3f boundMax, boundMin;
    GLVector3f center;
    STriangle* triangles;
    uint32_t trianglesCount;
    float maxSize;
} STrianglesInfo;

void calculateTriangleInfo(STriangle* triangles, uint32_t trianglesCount, STrianglesInfo* info) {
    info->trianglesCount = trianglesCount;
    info->triangles = triangles;
    if (trianglesCount == 0) {
        info->boundMin = info->boundMax = info->center = (GLVector3f){0, 0, 0};
        return;
    }
    float min[3], max[3];
    info->boundMin = info->boundMax = (GLVector3f){0, 0, 0};

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
    float wx = min[0] + max[0], wy = min[1] + max[1], wz = min[2] + max[2];
    info->center.x = wx / 2.0f;
    info->center.y = wy / 2.0f;
    info->center.z = wz / 2.0f;

    info->boundMin.x = min[0];
    info->boundMin.y = min[1];
    info->boundMin.z = min[2];

    info->boundMax.x = max[0];
    info->boundMax.y = max[1];
    info->boundMax.z = max[2];

    info->maxSize = wx;
    if (wy > info->maxSize)
        info->maxSize = wy;
    if (wz > info->maxSize)
        info->maxSize = wz;
}

void loadStlASCII(const char* fileName, STrianglesInfo* trianglesInfo) {
    const char* solid = "solid";
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

    if (!fileName) {
        fprintf(stderr, "Invalid file path\n");
        return;
    }

    FILE* file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file '%s'\n", fileName);
        return;
    }

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