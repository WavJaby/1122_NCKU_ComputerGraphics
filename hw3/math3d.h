#ifndef _MATH3D_LIBRARY__
#define _MATH3D_LIBRARY__

#include <math.h>
#include <memory.h>
#include <stdbool.h>

typedef float M3DVector3f[3];   // Vector of three floats (x, y, z)
typedef double M3DVector3d[3];  // Vector of three doubles (x, y, z)

typedef float M3DVector4f[4];   // Lesser used... Do we really need these?
typedef double M3DVector4d[4];  // Yes, occasionaly

typedef float M3DVector2f[2];   // 3D points = 3D Vectors, but we need a
typedef double M3DVector2d[2];  // 2D representations sometimes... (x,y) order

// 3x3 matrix - column major. X vector is 0, 1, 2, etc.
//		0	3	6
//		1	4	7
//		2	5	8
typedef float M3DMatrix33f[9];   // A 3 x 3 matrix, column major (floats) - OpenGL Style
typedef double M3DMatrix33d[9];  // A 3 x 3 matrix, column major (doubles) - OpenGL Style

// 4x4 matrix - column major. X vector is 0, 1, 2, etc.
//	0	4	8	12
//	1	5	9	13
//	2	6	10	14
//	3	7	11	15
typedef float M3DMatrix44f[16];   // A 4 X 4 matrix, column major (floats) - OpenGL style
typedef double M3DMatrix44d[16];  // A 4 x 4 matrix, column major (doubles) - OpenGL style

///////////////////////////////////////////////////////////////////////////////
// Useful constants
#define M3D_PI (3.14159265358979323846)
#define M3D_2PI (2.0 * M3D_PI)
#define M3D_PI_DIV_180 (0.017453292519943296)
#define M3D_INV_PI_DIV_180 (57.2957795130823229)

///////////////////////////////////////////////////////////////////////////////
// Useful shortcuts and macros
// Radians are king... but we need a way to swap back and forth
#define m3dDegToRad(x) ((x) * M3D_PI_DIV_180)
#define m3dRadToDeg(x) ((x) * M3D_INV_PI_DIV_180)

// Hour angles
#define m3dHrToDeg(x) ((x) * (1.0 / 15.0))
#define m3dHrToRad(x) m3dDegToRad(m3dHrToDeg(x))

#define m3dDegToHr(x) ((x) * 15.0)
#define m3dRadToHr(x) m3dDegToHr(m3dRadToDeg(x))

// 4x4 float
void m3dLoadIdentity44(M3DMatrix44f m) {
    // Don't be fooled, this is still column major
    static M3DMatrix44f identity_ = {1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f};

    memcpy(m, identity_, sizeof(M3DMatrix44f));
}

// Translate matrix. Only 4x4 matrices supported
static inline void m3dTranslateMatrix44(M3DMatrix44f m, float x, float y, float z) {
    m[12] += x;
    m[13] += y;
    m[14] += z;
}

// Scale matrix. Only 4x4 matrices supported
static inline void m3dScaleMatrix44(M3DMatrix44f m, float x, float y, float z) {
    m[0] *= x;
    m[5] *= y;
    m[10] *= z;
}

#define A(row, col) a[(col << 2) + row]
#define B(row, col) b[(col << 2) + row]
#define P(row, col) product[(col << 2) + row]

///////////////////////////////////////////////////////////////////////////////
// Multiply two 4x4 matricies
void m3dMatrixMultiply44(M3DMatrix44f product, const M3DMatrix44f a, const M3DMatrix44f b) {
    for (int i = 0; i < 4; i++) {
        float ai0 = A(i, 0), ai1 = A(i, 1), ai2 = A(i, 2), ai3 = A(i, 3);
        P(i, 0) = ai0 * B(0, 0) + ai1 * B(1, 0) + ai2 * B(2, 0) + ai3 * B(3, 0);
        P(i, 1) = ai0 * B(0, 1) + ai1 * B(1, 1) + ai2 * B(2, 1) + ai3 * B(3, 1);
        P(i, 2) = ai0 * B(0, 2) + ai1 * B(1, 2) + ai2 * B(2, 2) + ai3 * B(3, 2);
        P(i, 3) = ai0 * B(0, 3) + ai1 * B(1, 3) + ai2 * B(2, 3) + ai3 * B(3, 3);
    }
}

// Transpose/Invert - Only 4x4 matricies supported
#define TRANSPOSE44(dst, src)                        \
    {                                                \
        for (int j = 0; j < 4; j++) {                \
            for (int i = 0; i < 4; i++) {            \
                dst[(j * 4) + i] = src[(i * 4) + j]; \
            }                                        \
        }                                            \
    }

static inline void m3dTransposeMatrix44(M3DMatrix44f dst, const M3DMatrix44f src) {
    TRANSPOSE44(dst, src);
}

#endif