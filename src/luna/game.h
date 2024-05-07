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

/// @brief List of all video related settings
typedef struct {
	size_t width;
	size_t height;
	bool fullscreen;
} VideoSettingsList;

/// @brief Descriptor pointing to all game settings
typedef struct {
	InputSettingsList* inputSettingsList;
	VideoSettingsList* videoSettingsList;
} SettingsList;

/// @brief Game state
typedef struct {
	SceneList* sceneList;
	SettingsList* settingsList;
	ResourceList* resourceList;
	ResourceListDesc* resourceDesc;
	size_t numResourceDesc;
} Game;

extern Game* _luna_game_instance;

/// @brief Create a game instance using the provided settings and resource files.
/// @param _settingsList Structure containing all game settings
/// @param _resourceDesc Array of resource list descriptors
/// @param _numResource Size of resource list descrptor array
void CreateGame(SettingsList* _settingsList, ResourceListDesc* _resourceDesc, size_t _numResourceDesc);

/// @brief Deallocate a game instance.
void DestroyGame();

/// @brief Update the game state.
void UpdateGame();

/// @brief Load a new resource file by reference name.
/// @param _name Name field in the resource list descriptor
void LoadResourceFileByName(const char* _name);

/// @brief Load a new resource file by global index.
/// @param _idx Index in the global array of resource list descriptors
void LoadResourceFileByIndex(size_t _idx);

/// @brief Load the given resource file, unloading the old one.
/// @param _desc Resource list descriptor
void _LoadResourceFile(ResourceListDesc _desc);

/// @brief Populate the given settings structures with values from an INI file.
/// @param _settingsFile Settings file name
/// @param _targetSettingsList Pointer to destination settings list structure
int LoadSettingsFile(char* _settingsFile, SettingsList* _targetSettingsList);

/// @brief Create a settings structure populated with default values.
/// @return Pointer to setting structure
SettingsList* GenerateDefaultSettings();

#define LUNA_GAME _luna_game_instance
#define LUNA_SCENES _luna_game_instance->sceneList
#define LUNA_RESOURCES _luna_game_instance->resourceList
#define LUNA_SETTINGS _luna_game_instance->settingsList
#define LUNA_INPUTS _luna_game_instance->settingsList->inputSettingsList
#define LUNA_VIDEO _luna_game_instance->settingsList->videoSettingsList

#endif