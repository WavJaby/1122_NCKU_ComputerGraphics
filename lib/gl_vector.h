#ifndef __GL_VECTOR_H__
#define __GL_VECTOR_H__

#include <GL/glut.h>
#include <math.h>

#define printVector(message, glvec3) printf("%s(%.5f, %.5f, %.5f)\n", message, (glvec3)->x, (glvec3)->y, (glvec3)->z)

typedef struct GLVector3f {
    GLfloat x, y, z;
} GLVector3f;

static inline GLVector3f GLVector3Normalize(GLVector3f glvec3) {
    double length = sqrt(glvec3.x * glvec3.x + glvec3.y * glvec3.y + glvec3.z * glvec3.z);
    return (GLVector3f){glvec3.x / length, glvec3.y / length, glvec3.z / length};
}

static inline float GLVector3Length(GLVector3f glvec3) {
    return sqrt(glvec3.x * glvec3.x + glvec3.y * glvec3.y + glvec3.z * glvec3.z);
}

static inline void GLVector3NormalizeTo(GLVector3f* glvec3) {
    double length = sqrt(glvec3->x * glvec3->x + glvec3->y * glvec3->y + glvec3->z * glvec3->z);
    if (length == 0)
        return;
    glvec3->x /= length;
    glvec3->y /= length;
    glvec3->z /= length;
}

static inline GLVector3f GLVector3Cross(GLVector3f glvec3a, GLVector3f glvec3b) {
    return (GLVector3f){
        glvec3a.y * glvec3b.z - glvec3a.z * glvec3b.y,
        glvec3a.z * glvec3b.x - glvec3a.x * glvec3b.z,
        glvec3a.x * glvec3b.y - glvec3a.y * glvec3b.x,
    };
}

static inline void GLVector3AddTo(GLVector3f glvec3, GLVector3f* destGlvec3) {
    destGlvec3->x += glvec3.x;
    destGlvec3->y += glvec3.y;
    destGlvec3->z += glvec3.z;
}

static inline void GLVector3MinusTo(GLVector3f glvec3, GLVector3f* destGlvec3) {
    destGlvec3->x -= glvec3.x;
    destGlvec3->y -= glvec3.y;
    destGlvec3->z -= glvec3.z;
}

static inline void GLVector3MultiplyTo(GLVector3f glvec3, GLVector3f* destGlvec3) {
    destGlvec3->x *= glvec3.x;
    destGlvec3->y *= glvec3.y;
    destGlvec3->z *= glvec3.z;
}

static inline void GLVector3ScaleTo(GLfloat value, GLVector3f* glvec3) {
    glvec3->x *= value;
    glvec3->y *= value;
    glvec3->z *= value;
}

#endif