#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "gl_vector.h"
#include "WJCL/list/wjcl_list_t.h"

typedef struct game_object {
    GLVector3f position;
    GLVector3f rotation;
    GLVector3f scale;
    GLuint modelListId;
    ListT childs;
} GameObject;

GameObject* newGameObject(GLVector3f position, GLVector3f rotation, GLVector3f scale, GLuint modelListId) {
    GameObject* gameObject = (GameObject*)malloc(sizeof(GameObject));
    gameObject->position = position;
    gameObject->rotation = rotation;
    gameObject->scale = scale;
    gameObject->modelListId = modelListId;
    gameObject->childs = listT_create(GameObject*);
}

GameObject* newGameObjectDefault(GLuint modelListId) {
    GameObject* gameObject = (GameObject*)malloc(sizeof(GameObject));
    gameObject->position = (GLVector3f){};
    gameObject->rotation = (GLVector3f){};
    gameObject->scale = (GLVector3f){};
    gameObject->modelListId = modelListId;
    gameObject->childs = listT_create(GameObject*);
}

void gameobjectAddChild(GameObject* parent, GameObject* child) {
    listT_add(&parent->childs, child);
}

#endif