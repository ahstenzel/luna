#ifndef LUNA_GAME_H
#define LUNA_GAME_H
/**
 * game.h
 * Top level game state manager.
*/
#include "luna/common.h"
#include "luna/scene.h"
#include "luna/resource.h"

typedef struct {
	SceneList* sceneList;
	ResourceList* resourceList;
	size_t width;
	size_t height;
	char* name;
} Game;

typedef struct {
	size_t width;
	size_t height;
	char* name;
} GameDesc;

Game* CreateGame(GameDesc _desc);

void DestroyGame(Game* _game);

void UpdateGame(Game* _game);

void LoadResourceFile(Game* _game, const char* _resourceFile, const char* _resourcePassword);

#endif