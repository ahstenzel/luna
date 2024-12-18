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
#include "luna/camera.h"

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
	SpriteList* _spriteList;		// List of sprites
	SpriteList* _backgroundList;	// List of sprites to draw below everything else
	SpriteList* _foregroundList;	// List of sprites to draw on top of everything else
	CollisionList* _collisionList;	// List of collisions
	TilemapList* _tilemapList;		// List of tilemaps
	CameraList* _cameraList;		// List of cameras
	ScenePushFPtr _pushFPtr;		// Stack push callback
	SceneTopFPtr _topFPtr;			// Stack top callback
	SceneUpdateFPtr _updateFPtr;	// Stack update callback
	ScenePopFPtr _popFPtr;			// Stack pop callback
	const SceneID _id;				// Unique scene ID
} Scene;

/// @brief Descriptor for creating a scene
typedef struct {
	ScenePushFPtr pushFPtr;			// Stack push callback
	SceneTopFPtr topFPtr;			// Stack top callback
	SceneUpdateFPtr updateFPtr;		// Stack update callback
	ScenePopFPtr popFPtr;			// Stack pop callback
	bool depthSorting;				// Enable depth sorting for drawable objects
} SceneDesc;

/// @brief Stack of active scenes.
typedef struct {
	stack_t* _sceneStack;
	unordered_map_t* _scenes;
} SceneList;

/// @brief Iterator for scene objects.
typedef struct {
	SceneList* _list;			// Scene list to iterate through
	unordered_map_it_t* _ptr;	// Internal iterator reference
	SceneID id;					// Unique scene ID
	Scene* data;				// Scene data structure
} SceneListIt;

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

/// @brief Get the number of scenes in the list.
/// @param _list Scene list pointer
/// @return Number of scenes
size_t GetSceneListSize(SceneList* _list);

/// @brief Get the number of scenes currently in the stack.
/// @param _list Scene list pointer
/// @return Number of scenes
size_t GetSceneStackSize(SceneList* _list);

/// @brief Get an iterator for the list of scenes.
/// @param _list Scene list pointer
/// @return Scene iterator
SceneListIt* SceneListItBegin(SceneList* _list);

/// @brief Move the iterator to the next element.
/// @param _it Scene iterator
/// @return Updated scene iterator (or NULL)
SceneListIt* SceneListItNext(SceneListIt* _it);

/// @brief Create a new scene for the list.
/// @param _list Scene list pointer
/// @param _desc Scene descriptor
/// @return Scene ID (or ID_NULL on error)
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

/// @brief Pop the top scene off the stack and immediately push another one on.
/// @param _list Scene list pointer
/// @param _id Scene id
void GotoScene(SceneList* _list, SceneID _id);

/// @brief Get the id of the scene on top of the stack.
/// @param _list Scene list pointer
/// @return Scene id
SceneID GetTopScene(SceneList* _list);

/// @brief Get the sprite list for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Sprite list pointer
SpriteList* GetSceneSpriteList(SceneList* _list, SceneID _id);

/// @brief Get the background sprite list for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Sprite list pointer
SpriteList* GetSceneBackgroundSpriteList(SceneList* _list, SceneID _id);

/// @brief Get the foreground sprite list for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Sprite list pointer
SpriteList* GetSceneForegroundSpriteList(SceneList* _list, SceneID _id);

/// @brief Get the collision map for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Collision map pointer
CollisionList* GetSceneCollisionList(SceneList* _list, SceneID _id);

/// @brief Get the tilemap list for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Tilemap list pointer
TilemapList* GetSceneTilemapList(SceneList* _list, SceneID _id);

/// @brief Get the camera list for the given scene.
/// @param _list Scene list pointer
/// @param _id Scene id
/// @return Camera list pointer
CameraList* GetSceneCameraList(SceneList* _list, SceneID _id);

/// @brief Get the sprite list for the scene on top of the stack.
#define LUNA_SPRITES GetSceneSpriteList(LUNA_SCENES, GetTopScene(LUNA_SCENES))

/// @brief Get the background sprite list for the scene on top of the stack.
#define LUNA_BACKGROUNDS GetSceneBackgroundSpriteList(LUNA_SCENES, GetTopScene(LUNA_SCENES))

/// @brief Get the foreground sprite list for the scene on top of the stack.
#define LUNA_FOREGROUNDS GetSceneForegroundSpriteList(LUNA_SCENES, GetTopScene(LUNA_SCENES))

/// @brief Get the collision map for the scene on top of the stack.
#define LUNA_COLLISIONS GetSceneCollisionList(LUNA_SCENES, GetTopScene(LUNA_SCENES))

/// @brief Get the tilemap list for the scene on top of the stack.
#define LUNA_TILES GetSceneTilemapList(LUNA_SCENES, GetTopScene(LUNA_SCENES))

/// @brief Get the camera list for the scene on top of the stack.
#define LUNA_CAMERAS GetCameraTilemapList(LUNA_SCENES, GetTopScene(LUNA_SCENES))

#endif