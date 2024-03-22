#ifndef LUNA_GAME_H
#define LUNA_GAME_H
/**
 * game.h
 * Top level game state manager.
*/
#include "luna/common.h"
#include "luna/scene.h"
#include "luna/resource.h"
#include "luna/input.h"

/// @brief Game state
typedef struct {
	SceneList* sceneList;
	ResourceList* resourceList;
	InputList* inputList;
	size_t width;
	size_t height;
	char* name;
} Game;

/// @brief Descriptor for creating the game state
typedef struct {
	size_t width;
	size_t height;
	char* name;
} GameDesc;

extern Game* _luna_game_instance;

/// @brief Create a game instance.
/// @param _gameDesc Game descriptor
/// @param _resourceDesc Resource list descriptor
/// @param _inputDesc Array of input list descriptors
/// @param _inputNum Number of elements in the input descriptor array
void CreateGame(GameDesc _gameDesc, ResourceListDesc _resourceDesc, InputDesc* _inputDesc, size_t _inputNum);

/// @brief Deallocate a game instance.
void DestroyGame();

/// @brief Update the game state.
void UpdateGame();

/// @brief Load a new resource file.
/// @param _desc Resource list descriptor
void LoadResourceFile(ResourceListDesc _desc);

#define LUNA_GAME _luna_game_instance
#define LUNA_SCENES _luna_game_instance->sceneList
#define LUNA_RESOURCES _luna_game_instance->resourceList
#define LUNA_INPUTS _luna_game_instance->inputList

#endif