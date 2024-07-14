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
	size_t _width;							// Window width
	size_t _height;							// Window height
	bool _fullscreen;						// Fullscreen window
} VideoSettingsList;

/// @brief Structure containing all game settings
typedef struct {
	InputSettingsList* _inputSettingsList;	// List of all input settings
	VideoSettingsList* _videoSettingsList;	// List of all video settings
} SettingsList;

/// @brief Game state
typedef struct {
	SceneList* _sceneList;					// List of all scenes
	SettingsList* _settingsList;			// List of all settings
	ResourceList* _resourceList;			// List of all resources
	ResourceListDesc _resourceDesc;			// Resource list descriptors
} Game;

extern Game* _luna_game_instance;

/// @brief Create a game instance using the provided settings and resource files.
/// @param _settingsList Structure containing all game settings
/// @param _resourceDesc Resource list descriptors
void CreateGame(SettingsList* _settingsList, ResourceListDesc _resourceDesc);

/// @brief Deallocate a game instance.
void DestroyGame();

/// @brief Update the game state.
void UpdateGame();

/// @brief Populate the given settings structures with values from an INI file.
/// @param _settingsFile Settings file name
/// @param _targetSettingsList Pointer to destination settings list structure
int LoadSettingsFile(char* _settingsFile, SettingsList* _targetSettingsList);

/// @brief Create a settings structure populated with default values.
/// @return Pointer to setting structure
SettingsList* GenerateDefaultSettings();

#define LUNA_GAME _luna_game_instance
#define LUNA_SCENES _luna_game_instance->_sceneList
#define LUNA_RESOURCES _luna_game_instance->_resourceList
#define LUNA_SETTINGS _luna_game_instance->_settingsList
#define LUNA_INPUTS _luna_game_instance->_settingsList->_inputSettingsList
#define LUNA_VIDEO _luna_game_instance->_settingsList->_videoSettingsList

#endif