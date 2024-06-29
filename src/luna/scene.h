#ifndef LUNA_SCENE_H
#define LUNA_SCENE_H
/**
 * scene.h
 * Container of game objects.
*/
#include "luna/common.h"
#include "luna/sprite.h"
#include "luna/collision.h"
#include "luna/tile.h"

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
	Tilemap* tilemap;
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

/// @brief Create a new scene list.
/// @return Scene list pointer
SceneList* _CreateSceneList();

/// @brief Update all scenes in the list.
/// @param _list Scene list pointer
/// @param _dt Delta time factor
void _UpdateSceneList(SceneList* _list, float _dt);

/// @brief Draw the current scene.
/// @param _list Scene list pointer
void _DrawSceneList(SceneList* _list);

/// @brief Deallocate the scene list.
/// @param _list Scene list pointer
void _DestroySceneList(SceneList* _list);

/// @brief Create a new scene for the list.
/// @param _list Scene list pointer
/// @param _desc Scene descriptor
/// @return Scene ID
SceneID CreateScene(SceneList* _list, SceneDesc _desc);

/// @brief Destroy a scene in the list.
/// @param _list Scene list pointer
/// @param _id Scene id
void DestroyScene(SceneList* _list, SceneID _id);

/// @brief Push a new scene onto the stack.
/// @param _list Scene list pointer
/// @param _id Scene id
void PushScene(SceneList* _list, SceneID _id);

/// @brief Pop the scene off the stack.
/// @param _list Scene list pointer
void PopScene(SceneList* _list);

/// @brief Get the id of the scene on top of the stack.
/// @param _list Scene list pointer
/// @return Scene id
SceneID GetTopScene(SceneList* _list);

/// @brief Create a new tilemap for the given scene, overwriting any existing one.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @param _desc Tilemap descriptor
/// @return Tilemap object pointer
Tilemap* CreateSceneTilemap(SceneList* _list, SceneID _id, TilemapDesc _desc);

/// @brief Get the tilemap for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Tilemap object pointer
Tilemap* GetSceneTilemap(SceneList* _list, SceneID _id);

/// @brief Get the tilemap of the scene on top of the stack.
/// @param _list Scene list pointer
/// @return Tilemap object pointer
Tilemap* _GetTopSceneTilemap(SceneList* _list);

/// @brief Get the sprite list of the scene on top of the stack.
/// @param _list Scene list pointer
/// @return Sprite list pointer
SpriteList* _GetTopSceneSprites(SceneList* _list);

/// @brief Get the collision list of the scene on top of the stack.
/// @param _list Scene list pointer
/// @return Collision list pointer
CollisionList* _GetTopSceneCollisions(SceneList* _list);

#define LUNA_SPRITES _GetTopSceneSprites(LUNA_SCENES)
#define LUNA_COLLISIONS _GetTopSceneCollisions(LUNA_SCENES)
#define LUNA_TILES _GetTopSceneTilemap(LUNA_SCENES)

#endif