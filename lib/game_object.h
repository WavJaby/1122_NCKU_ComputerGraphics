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

    Matrix44f thisMatrix = identity;
    if (gameObject->modelListId) {
        glTranslatef(vx(gameObject->position), vy(gameObject->position), vz(gameObject->position));
        glRotatef(vx(gameObject->rotation), 1, 0, 0);
        glRotatef(vy(gameObject->rotation), 0, 1, 0);
        glRotatef(vz(gameObject->rotation), 0, 0, 1);
        // wUpdate matrix
        if (parentMat) {
            Matrix44f cache = identity, cache1 ;
            mat44fTranslate(thisMatrix, gameObject->position);
            mat44fMultiply(parentMat, thisMatrix, cache);

            mat44fRotationX(thisMatrix, vx(gameObject->rotation) * Deg2Rad);
            mat44fMultiply(cache, thisMatrix, cache1);
            mat44fRotationY(thisMatrix, vy(gameObject->rotation) * Deg2Rad);
            mat44fMultiply(cache1, thisMatrix, cache);
            mat44fRotationZ(cache1, vz(gameObject->rotation) * Deg2Rad);
            mat44fMultiply(cache, cache1, thisMatrix);
            mat44fGetPosition(thisMatrix, gameObject->globalPosition);
            mat44fGetEulerAngles(thisMatrix, gameObject->globalRotation);
            // printf(mat44PrintFmt("%.2f") "\n", mat44Print(parentMat));
        }
        glCallList(gameObject->modelListId);
    }
    listT_foreach(&gameObject->childs, GameObject*, i, {
        renderGameObject(i, parentMat ? thisMatrix : NULL);
    });
    glPopMatrix();
}

#endif