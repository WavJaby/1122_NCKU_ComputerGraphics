#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include <GL/glut.h>

#include "gl_vector.h"
#include "WJCL/list/wjcl_list_t.h"

typedef enum ColliderType {
    COLIDER_NONE,
    COLIDER_BOX,
    COLIDER_SPHERE,
} ColliderType;

typedef struct Collider {
    ColliderType type;
    Vector3f offset;
    Vector3f size;
} Collider;

typedef struct GameObject {
    Vector3f position;
    Vector3f globalPosition;
    Vector3f rotation;
    Vector3f globalRotation;
    Vector3f scale;
    Collider collider;
    GLuint modelListId;
    ListT childs;
} GameObject;

GameObject* newGameObjectPosRot(Vector3f position, Vector3f rotation, GLuint modelListId) {
    GameObject* gameObject = (GameObject*)malloc(sizeof(GameObject));
    vec3fSet(gameObject->position, position);
    vec3fSet(gameObject->rotation, rotation);
    vec3fSet(gameObject->scale, (Vector3f){1, 1, 1});
    gameObject->collider = (Collider){COLIDER_NONE};
    gameObject->modelListId = modelListId;
    gameObject->childs = listT_create(GameObject*);
    return gameObject;
}

GameObject* newGameObjectDefault(GLuint modelListId) {
    GameObject* gameObject = (GameObject*)malloc(sizeof(GameObject));
    vec3fSet(gameObject->position, (Vector3f){0, 0, 0});
    vec3fSet(gameObject->rotation, (Vector3f){0, 0, 0});
    vec3fSet(gameObject->scale, (Vector3f){1, 1, 1});
    gameObject->collider = (Collider){COLIDER_NONE};
    gameObject->modelListId = modelListId;
    gameObject->childs = listT_create(GameObject*);
    return gameObject;
}

void gameobjectAddChild(GameObject* parent, GameObject* child) {
    listT_add(&parent->childs, child);
}

void renderGameObject(GameObject* gameObject, Matrix44f parentMat) {
    // Render GameObject
    glPushMatrix();

    glTranslatef(vx(gameObject->position), vy(gameObject->position), vz(gameObject->position));
    glRotatef(vx(gameObject->rotation), 1, 0, 0);
    glRotatef(vy(gameObject->rotation), 0, 1, 0);
    glRotatef(vz(gameObject->rotation), 0, 0, 1);
    Matrix44f thisMatrix = identity;
    // wUpdate matrix
    if (parentMat) {
        Matrix44f cache = identity;
        mat44fTranslate(cache, gameObject->position);
        mat44fMultiply(parentMat, cache, thisMatrix);

        mat44fRotationX(cache, vx(gameObject->rotation) * Deg2Rad);
        mat44fMultiply(thisMatrix, cache, thisMatrix);
        mat44fRotationY(cache, vy(gameObject->rotation) * Deg2Rad);
        mat44fMultiply(thisMatrix, cache, thisMatrix);
        mat44fRotationZ(cache, vz(gameObject->rotation) * Deg2Rad);
        mat44fMultiply(thisMatrix, cache, thisMatrix);
        mat44fGetPosition(thisMatrix, gameObject->globalPosition);
        mat44fGetEulerAngles(thisMatrix, gameObject->globalRotation);
        // printf(mat44PrintFmt("%.2f") "\n", mat44Print(parentMat));
    }

    if (gameObject->modelListId)
        glCallList(gameObject->modelListId);
    listT_foreach(&gameObject->childs, GameObject*, i, {
        renderGameObject(i, parentMat ? thisMatrix : NULL);
    });
    glPopMatrix();
}

#endif