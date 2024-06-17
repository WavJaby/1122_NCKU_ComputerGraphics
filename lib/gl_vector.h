#ifndef __GL_VECTOR_H__
#define __GL_VECTOR_H__

#include <GL/glut.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846
#define Deg2Rad 0.017453292519943296
#define Rad2Deg 57.2957795130823229

#define vec3PrintFmt(format) "(" format ", " format ", " format ")"
#define vec3Print(vec3) vec3[0], vec3[1], vec3[2]

#define mat44PrintFmt(format) "[" format ", " format ", " format ", " format "\n " format ", " format ", " format ", " format "\n " format ", " format ", " format ", " format "\n " format ", " format ", " format ", " format "]"
#define mat44Print(mat44) mat44[0], mat44[4], mat44[8], mat44[12], mat44[1], mat44[5], mat44[9], mat44[13], mat44[2], mat44[6], mat44[10], mat44[14], mat44[3], mat44[7], mat44[11], mat44[15]

// Vector getter
#define vx(vector) vector[0]
#define vy(vector) vector[1]
#define vz(vector) vector[2]

// Vector float 3D
typedef float Vector3f[3];

#define vec3Clone(vector) \
    { vector[0], vector[1], vector[2] }

static inline void vec3fSet(Vector3f dest, Vector3f vec3) {
    memcpy(dest, vec3, 3 * sizeof(float));
}

static inline void vec3fZero(Vector3f dest) {
    dest[0] = 0;
    dest[1] = 0;
    dest[2] = 0;
}

static inline float vec3fLength(Vector3f vec3) {
    return sqrt(vx(vec3) * vx(vec3) + vy(vec3) * vy(vec3) + vz(vec3) * vz(vec3));
}

static inline float vec3fDistance(Vector3f a, Vector3f b) {
    Vector3f vec3 = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    return sqrt(vx(vec3) * vx(vec3) + vy(vec3) * vy(vec3) + vz(vec3) * vz(vec3));
}

static inline void vec3fNormalize(Vector3f vec3) {
    double length = sqrt(vx(vec3) * vx(vec3) + vy(vec3) * vy(vec3) + vz(vec3) * vz(vec3));
    if (length == 0) return;
    vx(vec3) /= length;
    vy(vec3) /= length;
    vz(vec3) /= length;
}

static inline void vec3fCross(Vector3f vec3a, Vector3f vec3b, Vector3f dest) {
    vx(dest) = vy(vec3a) * vz(vec3b) - vz(vec3a) * vy(vec3b);
    vy(dest) = vz(vec3a) * vx(vec3b) - vx(vec3a) * vz(vec3b);
    vz(dest) = vx(vec3a) * vy(vec3b) - vy(vec3a) * vx(vec3b);
}

static inline void vec3fAdd(Vector3f vec3, Vector3f dest) {
    vx(dest) += vx(vec3);
    vy(dest) += vy(vec3);
    vz(dest) += vz(vec3);
}

static inline void vec3fMinus(Vector3f vec3, Vector3f dest) {
    vx(dest) -= vx(vec3);
    vy(dest) -= vy(vec3);
    vz(dest) -= vz(vec3);
}

static inline void vec3fMultiply(Vector3f vec3, Vector3f dest) {
    vx(dest) *= vx(vec3);
    vy(dest) *= vy(vec3);
    vz(dest) *= vz(vec3);
}

static inline void vec3fScale(GLfloat value, Vector3f dest) {
    vx(dest) *= value;
    vy(dest) *= value;
    vz(dest) *= value;
}

// 4x4 matrix
//	0	4	8	12
//	1	5	9	13
//	2	6	10	14
//	3	7	11	15
typedef float Matrix44f[16];  // 4 X 4 matrix

#define identity              \
    { 1.0f, 0.0f, 0.0f, 0.0f, \
      0.0f, 1.0f, 0.0f, 0.0f, \
      0.0f, 0.0f, 1.0f, 0.0f, \
      0.0f, 0.0f, 0.0f, 1.0f }

#define  glvA(x, y) a[(y << 2) + x]
#define  glvB(x, y) b[(y << 2) + x]
#define  glvP(x, y) product[(y << 2) + x]

// Multiply two 4x4 matricies
void mat44fMultiply(const Matrix44f a, const Matrix44f b, Matrix44f product) {
    for (int i = 0; i < 4; i++) {
        float ai0 = glvA(i, 0), ai1 = glvA(i, 1), ai2 = glvA(i, 2), ai3 = glvA(i, 3);
        glvP(i, 0) = ai0 *  glvB(0, 0) + ai1 *  glvB(1, 0) + ai2 *  glvB(2, 0) + ai3 *  glvB(3, 0);
        glvP(i, 1) = ai0 *  glvB(0, 1) + ai1 *  glvB(1, 1) + ai2 *  glvB(2, 1) + ai3 *  glvB(3, 1);
        glvP(i, 2) = ai0 *  glvB(0, 2) + ai1 *  glvB(1, 2) + ai2 *  glvB(2, 2) + ai3 *  glvB(3, 2);
        glvP(i, 3) = ai0 *  glvB(0, 3) + ai1 *  glvB(1, 3) + ai2 *  glvB(2, 3) + ai3 *  glvB(3, 3);
    }
}

static inline void mat44fTranslate(Matrix44f m, const Vector3f vec) {
    m[12] += vx(vec);
    m[13] += vy(vec);
    m[14] += vz(vec);
}

void mat44fRotationX(Matrix44f a, double angle) {
    double s = sin(angle), c = cos(angle);

    glvA(0, 0) = 1;
    glvA(0, 1) = 0;
    glvA(0, 2) = 0;
    glvA(0, 3) = 0;

    glvA(1, 0) = 0;
    glvA(1, 1) = c;
    glvA(1, 2) = -s;
    glvA(1, 3) = 0;

    glvA(2, 0) = 0;
    glvA(2, 1) = s;
    glvA(2, 2) = c;
    glvA(2, 3) = 0;

    glvA(3, 0) = 0;
    glvA(3, 1) = 0;
    glvA(3, 2) = 0;
    glvA(3, 3) = 1;
}

void mat44fRotationY(Matrix44f a, double angle) {
    double s = sin(angle), c = cos(angle);

    glvA(0, 0) = c;
    glvA(0, 1) = 0;
    glvA(0, 2) = s;
    glvA(0, 3) = 0;

    glvA(1, 0) = 0;
    glvA(1, 1) = 1;
    glvA(1, 2) = 0;
    glvA(1, 3) = 0;

    glvA(2, 0) = -s;
    glvA(2, 1) = 0;
    glvA(2, 2) = c;
    glvA(2, 3) = 0;

    glvA(3, 0) = 0;
    glvA(3, 1) = 0;
    glvA(3, 2) = 0;
    glvA(3, 3) = 1;
}

void mat44fRotationZ(Matrix44f a, double angle) {
    double s = sin(angle), c = cos(angle);

    glvA(0, 0) = c;
    glvA(0, 1) = -s;
    glvA(0, 2) = 0;
    glvA(0, 3) = 0;

    glvA(1, 0) = s;
    glvA(1, 1) = c;
    glvA(1, 2) = 0;
    glvA(1, 3) = 0;

    glvA(2, 0) = 0;
    glvA(2, 1) = 0;
    glvA(2, 2) = 1;
    glvA(2, 3) = 0;

    glvA(3, 0) = 0;
    glvA(3, 1) = 0;
    glvA(3, 2) = 0;
    glvA(3, 3) = 1;
}

static inline void mat44fGetPosition(Matrix44f m, Vector3f dest) {
    vx(dest) = m[12];
    vy(dest) = m[13];
    vz(dest) = m[14];
}

static inline void mat44fGetEulerAngles(const Matrix44f a, Vector3f dest) {
    dest[0] = atan2(-glvA(1, 2), glvA(2, 2));
    float cosYangle = sqrt(pow(glvA(0, 0), 2) + pow(glvA(0, 1), 2));
    dest[1] = atan2(glvA(0, 2), cosYangle);
    float sinXangle = sin(dest[0]);
    float cosXangle = cos(dest[0]);
    dest[2] = atan2(cosXangle * glvA(1, 0) + sinXangle * glvA(2, 0), cosXangle * glvA(1, 1) + sinXangle * glvA(2, 1));
}
#endif