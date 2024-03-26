#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/gl_vector.h"

typedef struct STriangle {
    float normal[3], a[3], b[3], c[3];
} STriangle;

STriangle* loadStlASCII(const char* pPathname, uint32_t* nTriangles) {
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

    if (!pPathname) {
        fprintf(stderr, "Invalid file path\n");
        return NULL;
    }

    FILE* file = fopen(pPathname, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file '%s'\n", pPathname);
        return NULL;
    }

    char str[256];
    if (!fgets(str, sizeof(str), file) || strncmp(str, solid, strlen(solid)) != 0) {
        fprintf(stderr, "Invalid STL file\n");
        fclose(file);
        return NULL;
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
    *nTriangles = listSize;
    return triList;
ERROR_HANDLE:
    *nTriangles = listSize;
    free(triList);
    fclose(file);
    return NULL;
}

STriangle* loadStlBinary(const char* pPathname, uint32_t* nTriangles) {
    if (!pPathname) {
        fprintf(stderr, "Invalid file path\n");
        return NULL;
    }

    FILE* file = fopen(pPathname, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file '%s'\n", pPathname);
        return NULL;
    }

    uint8_t tmpBuff[80];
    size_t len = fread(tmpBuff, 1, sizeof(tmpBuff), file);
    if (len != sizeof(tmpBuff)) {
        fprintf(stderr, "Invalid STL file: Bad header\n");
        goto ERROR_HANDLE;
    }

    // Read triangle count
    fread(nTriangles, 4, 1, file);
    if (nTriangles < 0) {
        fprintf(stderr, "Invalid STL file: Triangle count %d\n", *nTriangles);
        goto ERROR_HANDLE;
    }

    // Allocate array
    STriangle* triList = (STriangle*)malloc(sizeof(STriangle) * (*nTriangles));

    for (size_t i = 0; i < (*nTriangles); i++) {
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

        uint16_t attrCount = *(uint16_t*)(tmpBuff + 48);
    }

    fclose(file);
    return triList;
ERROR_HANDLE:
    *nTriangles = 0;
    free(triList);
    fclose(file);
    return NULL;
}