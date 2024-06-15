#ifndef LINMATH_H
#define LINMATH_H

#include <math.h>
#include <string.h>

#define M_E 2.7182818284590452354         /* e */
#define M_LOG2E 1.4426950408889634074     /* log_2 e */
#define M_LOG10E 0.43429448190325182765   /* log_10 e */
#define M_LN2 0.69314718055994530942      /* log_e 2 */
#define M_LN10 2.30258509299404568402     /* log_e 10 */
#define M_PI 3.14159265358979323846       /* pi */
#define M_PI_2 1.57079632679489661923     /* pi/2 */
#define M_PI_4 0.78539816339744830962     /* pi/4 */
#define M_1_PI 0.31830988618379067154     /* 1/pi */
#define M_2_PI 0.63661977236758134308     /* 2/pi */
#define M_2_SQRTPI 1.12837916709551257390 /* 2/sqrt(pi) */
#define M_SQRT2 1.41421356237309504880    /* sqrt(2) */
#define M_SQRT1_2 0.70710678118654752440  /* 1/sqrt(2) */

#define M_DEG_2_RAD 0.01745329251994329576
#define M_RAD_2_DEG 57.29577951308232286464

#ifdef LINMATH_NO_INLINE
#define LINMATH_H_FUNC static
#else
#define LINMATH_H_FUNC static inline
#endif

#define LINMATH_H_DEFINE_VEC(n)                                                   \
    typedef float vec##n[n];                                                      \
    LINMATH_H_FUNC void vec##n##_add(vec##n r, vec##n const a, vec##n const b) {  \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            r[i] = a[i] + b[i];                                                   \
    }                                                                             \
    LINMATH_H_FUNC void vec##n##_sub(vec##n r, vec##n const a, vec##n const b) {  \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            r[i] = a[i] - b[i];                                                   \
    }                                                                             \
    LINMATH_H_FUNC void vec##n##_scale(vec##n r, vec##n const v, float const s) { \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            r[i] = v[i] * s;                                                      \
    }                                                                             \
    LINMATH_H_FUNC float vec##n##_mul_inner(vec##n const a, vec##n const b) {     \
        float p = 0.f;                                                            \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            p += b[i] * a[i];                                                     \
        return p;                                                                 \
    }                                                                             \
    LINMATH_H_FUNC float vec##n##_len(vec##n const v) {                           \
        return sqrtf(vec##n##_mul_inner(v, v));                                   \
    }                                                                             \
    LINMATH_H_FUNC void vec##n##_norm(vec##n r, vec##n const v) {                 \
        float k = 1.f / vec##n##_len(v);                                          \
        vec##n##_scale(r, v, k);                                                  \
    }                                                                             \
    LINMATH_H_FUNC void vec##n##_min(vec##n r, vec##n const a, vec##n const b) {  \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            r[i] = a[i] < b[i] ? a[i] : b[i];                                     \
    }                                                                             \
    LINMATH_H_FUNC void vec##n##_max(vec##n r, vec##n const a, vec##n const b) {  \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            r[i] = a[i] > b[i] ? a[i] : b[i];                                     \
    }                                                                             \
    LINMATH_H_FUNC void vec##n##_dup(vec##n r, vec##n const src) {                \
        int i;                                                                    \
        for (i = 0; i < n; ++i)                                                   \
            r[i] = src[i];                                                        \
    }

LINMATH_H_DEFINE_VEC(2)
LINMATH_H_DEFINE_VEC(3)
LINMATH_H_DEFINE_VEC(4)

const vec3 vec3_up = {0, 1, 0};

LINMATH_H_FUNC void vec3_mul_cross(vec3 r, vec3 const a, vec3 const b) {
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
}

LINMATH_H_FUNC void vec3_reflect(vec3 r, vec3 const v, vec3 const n) {
    float p = 2.f * vec3_mul_inner(v, n);
    int i;
    for (i = 0; i < 3; ++i)
        r[i] = v[i] - p * n[i];
}

LINMATH_H_FUNC void vec4_mul_cross(vec4 r, vec4 const a, vec4 const b) {
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
    r[3] = 1.f;
}

LINMATH_H_FUNC void vec4_reflect(vec4 r, vec4 const v, vec4 const n) {
    float p = 2.f * vec4_mul_inner(v, n);
    int i;
    for (i = 0; i < 4; ++i)
        r[i] = v[i] - p * n[i];
}

typedef vec4 mat4x4[4];

#define mat4x4_identity_c \
    {                     \
        {1, 0, 0, 0},     \
            {0, 1, 0, 0}, \
            {0, 0, 1, 0}, \
            {0, 0, 0, 1}, \
    }

const mat4x4 mat4x4_identity = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};

LINMATH_H_FUNC void mat4x4_identity_create(mat4x4 M) {
    memcpy(M, mat4x4_identity, sizeof(mat4x4));
}
LINMATH_H_FUNC void mat4x4_dup(mat4x4 M, mat4x4 const N) {
    memcpy(M, N, sizeof(mat4x4));
}
LINMATH_H_FUNC void mat4x4_row(vec4 r, mat4x4 const M, int i) {
    int k;
    for (k = 0; k < 4; ++k)
        r[k] = M[k][i];
}
LINMATH_H_FUNC void mat4x4_col(vec4 r, mat4x4 const M, int i) {
    int k;
    for (k = 0; k < 4; ++k)
        r[k] = M[i][k];
}
LINMATH_H_FUNC void mat4x4_transpose(mat4x4 M, mat4x4 const N) {
    // Note: if M and N are the same, the user has to
    // explicitly make a copy of M and set it to N.
    int i, j;
    for (j = 0; j < 4; ++j)
        for (i = 0; i < 4; ++i)
            M[i][j] = N[j][i];
}
LINMATH_H_FUNC void mat4x4_add(mat4x4 M, mat4x4 const a, mat4x4 const b) {
    int i;
    for (i = 0; i < 4; ++i)
        vec4_add(M[i], a[i], b[i]);
}
LINMATH_H_FUNC void mat4x4_sub(mat4x4 M, mat4x4 const a, mat4x4 const b) {
    int i;
    for (i = 0; i < 4; ++i)
        vec4_sub(M[i], a[i], b[i]);
}
LINMATH_H_FUNC void mat4x4_scale(mat4x4 M, mat4x4 const a, float k) {
    int i;
    for (i = 0; i < 4; ++i)
        vec4_scale(M[i], a[i], k);
}
LINMATH_H_FUNC void mat4x4_scale_aniso(mat4x4 M, mat4x4 const a, float x, float y, float z) {
    vec4_scale(M[0], a[0], x);
    vec4_scale(M[1], a[1], y);
    vec4_scale(M[2], a[2], z);
    vec4_dup(M[3], a[3]);
}
LINMATH_H_FUNC void mat4x4_mul(mat4x4 M, mat4x4 const a, mat4x4 const b) {
    for (size_t i = 0; i < 4; i++) {
        float ai0 = a[0][i], ai1 = a[1][i], ai2 = a[2][i], ai3 = a[3][i];
        M[0][i] = ai0 * b[0][0] + ai1 * b[0][1] + ai2 * b[0][2] + ai3 * b[0][3];
        M[1][i] = ai0 * b[1][0] + ai1 * b[1][1] + ai2 * b[1][2] + ai3 * b[1][3];
        M[2][i] = ai0 * b[2][0] + ai1 * b[2][1] + ai2 * b[2][2] + ai3 * b[2][3];
        M[3][i] = ai0 * b[3][0] + ai1 * b[3][1] + ai2 * b[3][2] + ai3 * b[3][3];
    }
}
LINMATH_H_FUNC void mat4x4_mul_vec4(vec4 r, mat4x4 const M, vec4 const v) {
    int i, j;
    for (j = 0; j < 4; ++j) {
        r[j] = 0.f;
        for (i = 0; i < 4; ++i)
            r[j] += M[i][j] * v[i];
    }
}
LINMATH_H_FUNC void mat4x4_translate_create(mat4x4 T, float x, float y, float z) {
    mat4x4_identity_create(T);
    T[3][0] = x;
    T[3][1] = y;
    T[3][2] = z;
}
LINMATH_H_FUNC void mat4x4_translate_in_place(mat4x4 M, float x, float y, float z) {
    vec4 t = {x, y, z, 0};
    vec4 r;
    int i;
    for (i = 0; i < 4; ++i) {
        mat4x4_row(r, M, i);
        M[3][i] += vec4_mul_inner(r, t);
    }
}
LINMATH_H_FUNC void mat4x4_from_vec3_mul_outer(mat4x4 M, vec3 const a, vec3 const b) {
    int i, j;
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            M[i][j] = i < 3 && j < 3 ? a[i] * b[j] : 0.f;
}
LINMATH_H_FUNC void mat4x4_rotate(mat4x4 R, mat4x4 const M, float x, float y, float z, float angle) {
    vec3 u = {x, y, z};

    if (vec3_len(u) > 1e-4) {
        float s = sinf(angle);
        float c = cosf(angle);
        vec3_norm(u, u);
        mat4x4 T;
        mat4x4_from_vec3_mul_outer(T, u, u);

        mat4x4 S = {
            {0, u[2], -u[1], 0},
            {-u[2], 0, u[0], 0},
            {u[1], -u[0], 0, 0},
            {0, 0, 0, 0}};
        mat4x4_scale(S, S, s);

        mat4x4 C;
        mat4x4_identity_create(C);
        mat4x4_sub(C, C, T);

        mat4x4_scale(C, C, c);

        mat4x4_add(T, T, C);
        mat4x4_add(T, T, S);

        T[3][3] = 1.f;
        mat4x4_mul(R, M, T);
    } else {
        mat4x4_dup(R, M);
    }
}
LINMATH_H_FUNC void mat4x4_rotate_X(mat4x4 Q, mat4x4 const M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
        {1.f, 0.f, 0.f, 0.f},
        {0.f, c, s, 0.f},
        {0.f, -s, c, 0.f},
        {0.f, 0.f, 0.f, 1.f}};
    mat4x4_mul(Q, M, R);
}
LINMATH_H_FUNC void mat4x4_rotate_Y(mat4x4 Q, mat4x4 const M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
        {c, 0.f, -s, 0.f},
        {0.f, 1.f, 0.f, 0.f},
        {s, 0.f, c, 0.f},
        {0.f, 0.f, 0.f, 1.f}};
    mat4x4_mul(Q, M, R);
}
LINMATH_H_FUNC void mat4x4_rotate_Z(mat4x4 Q, mat4x4 const M, float angle) {
    float s = sinf(angle);
    float c = cosf(angle);
    mat4x4 R = {
        {c, s, 0.f, 0.f},
        {-s, c, 0.f, 0.f},
        {0.f, 0.f, 1.f, 0.f},
        {0.f, 0.f, 0.f, 1.f}};
    mat4x4_mul(Q, M, R);
}
LINMATH_H_FUNC void mat4x4_rotate_create(mat4x4 R, float x, float y, float z, float angle) {
    mat4x4_rotate(R, mat4x4_identity, x, y, z, angle);
}
LINMATH_H_FUNC void mat4x4_rotate_create_X(mat4x4 R, float angle) {
    mat4x4_rotate_X(R, mat4x4_identity, angle);
}
LINMATH_H_FUNC void mat4x4_rotate_create_Y(mat4x4 R, float angle) {
    mat4x4_rotate_Y(R, mat4x4_identity, angle);
}
LINMATH_H_FUNC void mat4x4_rotate_create_Z(mat4x4 R, float angle) {
    mat4x4_rotate_Z(R, mat4x4_identity, angle);
}
LINMATH_H_FUNC void mat4x4_invert(mat4x4 T, mat4x4 const M) {
    float s[6];
    float c[6];
    s[0] = M[0][0] * M[1][1] - M[1][0] * M[0][1];
    s[1] = M[0][0] * M[1][2] - M[1][0] * M[0][2];
    s[2] = M[0][0] * M[1][3] - M[1][0] * M[0][3];
    s[3] = M[0][1] * M[1][2] - M[1][1] * M[0][2];
    s[4] = M[0][1] * M[1][3] - M[1][1] * M[0][3];
    s[5] = M[0][2] * M[1][3] - M[1][2] * M[0][3];

    c[0] = M[2][0] * M[3][1] - M[3][0] * M[2][1];
    c[1] = M[2][0] * M[3][2] - M[3][0] * M[2][2];
    c[2] = M[2][0] * M[3][3] - M[3][0] * M[2][3];
    c[3] = M[2][1] * M[3][2] - M[3][1] * M[2][2];
    c[4] = M[2][1] * M[3][3] - M[3][1] * M[2][3];
    c[5] = M[2][2] * M[3][3] - M[3][2] * M[2][3];

    /* Assumes it is invertible */
    float idet = 1.0f / (s[0] * c[5] - s[1] * c[4] + s[2] * c[3] + s[3] * c[2] - s[4] * c[1] + s[5] * c[0]);

    T[0][0] = (M[1][1] * c[5] - M[1][2] * c[4] + M[1][3] * c[3]) * idet;
    T[0][1] = (-M[0][1] * c[5] + M[0][2] * c[4] - M[0][3] * c[3]) * idet;
    T[0][2] = (M[3][1] * s[5] - M[3][2] * s[4] + M[3][3] * s[3]) * idet;
    T[0][3] = (-M[2][1] * s[5] + M[2][2] * s[4] - M[2][3] * s[3]) * idet;

    T[1][0] = (-M[1][0] * c[5] + M[1][2] * c[2] - M[1][3] * c[1]) * idet;
    T[1][1] = (M[0][0] * c[5] - M[0][2] * c[2] + M[0][3] * c[1]) * idet;
    T[1][2] = (-M[3][0] * s[5] + M[3][2] * s[2] - M[3][3] * s[1]) * idet;
    T[1][3] = (M[2][0] * s[5] - M[2][2] * s[2] + M[2][3] * s[1]) * idet;

    T[2][0] = (M[1][0] * c[4] - M[1][1] * c[2] + M[1][3] * c[0]) * idet;
    T[2][1] = (-M[0][0] * c[4] + M[0][1] * c[2] - M[0][3] * c[0]) * idet;
    T[2][2] = (M[3][0] * s[4] - M[3][1] * s[2] + M[3][3] * s[0]) * idet;
    T[2][3] = (-M[2][0] * s[4] + M[2][1] * s[2] - M[2][3] * s[0]) * idet;

    T[3][0] = (-M[1][0] * c[3] + M[1][1] * c[1] - M[1][2] * c[0]) * idet;
    T[3][1] = (M[0][0] * c[3] - M[0][1] * c[1] + M[0][2] * c[0]) * idet;
    T[3][2] = (-M[3][0] * s[3] + M[3][1] * s[1] - M[3][2] * s[0]) * idet;
    T[3][3] = (M[2][0] * s[3] - M[2][1] * s[1] + M[2][2] * s[0]) * idet;
}
LINMATH_H_FUNC void mat4x4_orthonormalize(mat4x4 R, mat4x4 const M) {
    mat4x4_dup(R, M);
    float s = 1.f;
    vec3 h;

    vec3_norm(R[2], R[2]);

    s = vec3_mul_inner(R[1], R[2]);
    vec3_scale(h, R[2], s);
    vec3_sub(R[1], R[1], h);
    vec3_norm(R[1], R[1]);

    s = vec3_mul_inner(R[0], R[2]);
    vec3_scale(h, R[2], s);
    vec3_sub(R[0], R[0], h);

    s = vec3_mul_inner(R[0], R[1]);
    vec3_scale(h, R[1], s);
    vec3_sub(R[0], R[0], h);
    vec3_norm(R[0], R[0]);
}

LINMATH_H_FUNC void mat4x4_frustum(mat4x4 M, float l, float r, float b, float t, float n, float f) {
    M[0][0] = 2.f * n / (r - l);
    M[0][1] = M[0][2] = M[0][3] = 0.f;

    M[1][1] = 2.f * n / (t - b);
    M[1][0] = M[1][2] = M[1][3] = 0.f;

    M[2][0] = (r + l) / (r - l);
    M[2][1] = (t + b) / (t - b);
    M[2][2] = -(f + n) / (f - n);
    M[2][3] = -1.f;

    M[3][2] = -2.f * (f * n) / (f - n);
    M[3][0] = M[3][1] = M[3][3] = 0.f;
}
LINMATH_H_FUNC void mat4x4_ortho(mat4x4 M, float l, float r, float b, float t, float n, float f) {
    M[0][0] = 2.f / (r - l);
    M[0][1] = M[0][2] = M[0][3] = 0.f;

    M[1][1] = 2.f / (t - b);
    M[1][0] = M[1][2] = M[1][3] = 0.f;

    M[2][2] = -2.f / (f - n);
    M[2][0] = M[2][1] = M[2][3] = 0.f;

    M[3][0] = -(r + l) / (r - l);
    M[3][1] = -(t + b) / (t - b);
    M[3][2] = -(f + n) / (f - n);
    M[3][3] = 1.f;
}
LINMATH_H_FUNC void mat4x4_perspective(mat4x4 m, float y_fov, float aspect, float n, float f) {
    /* NOTE: Degrees are an unhandy unit to work with.
     * linmath.h uses radians for everything! */
    float const a = 1.f / tanf(y_fov / 2.f);

    m[0][0] = a / aspect;
    m[0][1] = 0.f;
    m[0][2] = 0.f;
    m[0][3] = 0.f;

    m[1][0] = 0.f;
    m[1][1] = a;
    m[1][2] = 0.f;
    m[1][3] = 0.f;

    m[2][0] = 0.f;
    m[2][1] = 0.f;
    m[2][2] = -((f + n) / (f - n));
    m[2][3] = -1.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = -((2.f * f * n) / (f - n));
    m[3][3] = 0.f;
}
LINMATH_H_FUNC void mat4x4_look_at(mat4x4 m, vec3 const eye, vec3 const center, vec3 const up) {
    /* Adapted from Android's OpenGL Matrix.java.                        */
    /* See the OpenGL GLUT documentation for gluLookAt for a description */
    /* of the algorithm. We implement it in a straightforward way:       */

    /* TODO: The negation of of can be spared by swapping the order of
     *       operands in the following cross products in the right way. */
    vec3 f;
    vec3_sub(f, center, eye);
    vec3_norm(f, f);

    vec3 s;
    vec3_mul_cross(s, f, up);
    vec3_norm(s, s);

    vec3 t;
    vec3_mul_cross(t, s, f);

    m[0][0] = s[0];
    m[0][1] = t[0];
    m[0][2] = -f[0];
    m[0][3] = 0.f;

    m[1][0] = s[1];
    m[1][1] = t[1];
    m[1][2] = -f[1];
    m[1][3] = 0.f;

    m[2][0] = s[2];
    m[2][1] = t[2];
    m[2][2] = -f[2];
    m[2][3] = 0.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = 0.f;
    m[3][3] = 1.f;

    mat4x4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);
}

typedef float quat[4];
#define quat_add vec4_add
#define quat_sub vec4_sub
#define quat_norm vec4_norm
#define quat_scale vec4_scale
#define quat_mul_inner vec4_mul_inner

LINMATH_H_FUNC void quat_identity(quat q) {
    q[0] = q[1] = q[2] = 0.f;
    q[3] = 1.f;
}
LINMATH_H_FUNC void quat_mul(quat r, quat const p, quat const q) {
    vec3 w, tmp;

    vec3_mul_cross(tmp, p, q);
    vec3_scale(w, p, q[3]);
    vec3_add(tmp, tmp, w);
    vec3_scale(w, q, p[3]);
    vec3_add(tmp, tmp, w);

    vec3_dup(r, tmp);
    r[3] = p[3] * q[3] - vec3_mul_inner(p, q);
}
LINMATH_H_FUNC void quat_conj(quat r, quat const q) {
    int i;
    for (i = 0; i < 3; ++i)
        r[i] = -q[i];
    r[3] = q[3];
}
LINMATH_H_FUNC void quat_rotate(quat r, float angle, vec3 const axis) {
    vec3 axis_norm;
    vec3_norm(axis_norm, axis);
    float s = sinf(angle / 2);
    float c = cosf(angle / 2);
    vec3_scale(r, axis_norm, s);
    r[3] = c;
}
LINMATH_H_FUNC void quat_mul_vec3(vec3 r, quat const q, vec3 const v) {
    /*
     * Method by Fabian 'ryg' Giessen (of Farbrausch)
    t = 2 * cross(q.xyz, v)
    v' = v + q.w * t + cross(q.xyz, t)
     */
    vec3 t;
    vec3 q_xyz = {q[0], q[1], q[2]};
    vec3 u = {q[0], q[1], q[2]};

    vec3_mul_cross(t, q_xyz, v);
    vec3_scale(t, t, 2);

    vec3_mul_cross(u, q_xyz, t);
    vec3_scale(t, t, q[3]);

    vec3_add(r, v, t);
    vec3_add(r, r, u);
}
LINMATH_H_FUNC void mat4x4_from_quat(mat4x4 M, quat const q) {
    float a = q[3];
    float b = q[0];
    float c = q[1];
    float d = q[2];
    float a2 = a * a;
    float b2 = b * b;
    float c2 = c * c;
    float d2 = d * d;

    M[0][0] = a2 + b2 - c2 - d2;
    M[0][1] = 2.f * (b * c + a * d);
    M[0][2] = 2.f * (b * d - a * c);
    M[0][3] = 0.f;

    M[1][0] = 2 * (b * c - a * d);
    M[1][1] = a2 - b2 + c2 - d2;
    M[1][2] = 2.f * (c * d + a * b);
    M[1][3] = 0.f;

    M[2][0] = 2.f * (b * d + a * c);
    M[2][1] = 2.f * (c * d - a * b);
    M[2][2] = a2 - b2 - c2 + d2;
    M[2][3] = 0.f;

    M[3][0] = M[3][1] = M[3][2] = 0.f;
    M[3][3] = 1.f;
}

LINMATH_H_FUNC void mat4x4o_mul_quat(mat4x4 R, mat4x4 const M, quat const q) {
    /*  XXX: The way this is written only works for orthogonal matrices. */
    /* TODO: Take care of non-orthogonal case. */
    quat_mul_vec3(R[0], q, M[0]);
    quat_mul_vec3(R[1], q, M[1]);
    quat_mul_vec3(R[2], q, M[2]);

    R[3][0] = R[3][1] = R[3][2] = 0.f;
    R[0][3] = M[0][3];
    R[1][3] = M[1][3];
    R[2][3] = M[2][3];
    R[3][3] = M[3][3];  // typically 1.0, but here we make it general
}
LINMATH_H_FUNC void quat_from_mat4x4(quat q, mat4x4 const M) {
    float r = 0.f;
    int i;

    int perm[] = {0, 1, 2, 0, 1};
    int *p = perm;

    for (i = 0; i < 3; i++) {
        float m = M[i][i];
        if (m < r)
            continue;
        m = r;
        p = &perm[i];
    }

    r = sqrtf(1.f + M[p[0]][p[0]] - M[p[1]][p[1]] - M[p[2]][p[2]]);

    if (r < 1e-6) {
        q[0] = 1.f;
        q[1] = q[2] = q[3] = 0.f;
        return;
    }

    q[0] = r / 2.f;
    q[1] = (M[p[0]][p[1]] - M[p[1]][p[0]]) / (2.f * r);
    q[2] = (M[p[2]][p[0]] - M[p[0]][p[2]]) / (2.f * r);
    q[3] = (M[p[2]][p[1]] - M[p[1]][p[2]]) / (2.f * r);
}

LINMATH_H_FUNC void mat4x4_arcball(mat4x4 R, mat4x4 const M, vec2 const _a, vec2 const _b, float s) {
    vec2 a;
    memcpy(a, _a, sizeof(a));
    vec2 b;
    memcpy(b, _b, sizeof(b));

    float z_a = 0.;
    float z_b = 0.;

    if (vec2_len(a) < 1.) {
        z_a = sqrtf(1. - vec2_mul_inner(a, a));
    } else {
        vec2_norm(a, a);
    }

    if (vec2_len(b) < 1.) {
        z_b = sqrtf(1. - vec2_mul_inner(b, b));
    } else {
        vec2_norm(b, b);
    }

    vec3 a_ = {a[0], a[1], z_a};
    vec3 b_ = {b[0], b[1], z_b};

    vec3 c_;
    vec3_mul_cross(c_, a_, b_);

    float const angle = acos(vec3_mul_inner(a_, b_)) * s;
    mat4x4_rotate(R, M, c_[0], c_[1], c_[2], angle);
}
#endif
