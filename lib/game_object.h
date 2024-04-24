#include "gl_vector.h"

typedef struct game_object {
    GLVector3f position;
    GLVector3f rotation;
    GLVector3f scale;
    GLuint modelDispListId;
    struct game_object* childs;
} GameObject;