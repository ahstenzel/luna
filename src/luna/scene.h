#ifndef LUNA_SCENE_H
#define LUNA_SCENE_H
/**
 * scene.h
 * Container of game objects.
*/
#include "luna/common.h"
#include "luna/sprite.h"
#include "luna/collision.h"

/// @brief Unique scene identifier
typedef _LUNA_ID_TYPE SceneID;

/// @brief Function to run when pushing a scene onto the stack.
typedef void (*ScenePushFPtr)(SceneID);

/// @brief Function to run when the scene becomes the top scene of the stack.
typedef void (*SceneTopFPtr)(SceneID);

/// @brief Function to run on the top scene every frame.
typedef void (*SceneUpdateFPtr)(SceneID, float);

/// @brief Function to run when popping a scene from the stack.
typedef void (*ScenePopFPtr)(SceneID);

/// @brief Individual game scene
typedef struct {
	SpriteList* spriteList;
	CollisionList* collisionMap;
	ScenePushFPtr pushFPtr;
	SceneTopFPtr topFPtr;
	SceneUpdateFPtr updateFPtr;
	ScenePopFPtr popFPtr;
	SceneID id;
} Scene;

/// @brief Descriptor for creating a scene
typedef struct {
	ScenePushFPtr pushFPtr;
	SceneTopFPtr topFPtr;
	SceneUpdateFPtr updateFPtr;
	ScenePopFPtr popFPtr;
	bool depthSorting;
} SceneDesc;

/// @brief Stack of active scenes.
typedef struct {
	stack_t* sceneStack;
	unordered_map_t* scenes;
} SceneList;

SceneList* CreateSceneList();

void UpdateSceneList(SceneList* _list, float _dt);

void DrawSceneList(SceneList* _list);

void DestroySceneList(SceneList* _list);

SceneID CreateScene(SceneList* _list, SceneDesc _desc);

void DestroyScene(SceneList* _list, SceneID _id);

void PushScene(SceneList* _list, SceneID _id);

void PopScene(SceneList* _list);

SceneID TopScene(SceneList* _list);

#endif