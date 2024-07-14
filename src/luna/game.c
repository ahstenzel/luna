#include "luna/game.h"
#include "luna/defaults.h"

Game* _luna_game_instance = NULL;

void CreateGame(SettingsList* _settingsList, ResourceListDesc _resourceDesc) {
	LUNA_RETURN_CLEAR;
	LUNA_DEBUG_LOG("===Creating game state===");
	Game* game = calloc(1, sizeof *game);
	if (!game) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate game state!");
		goto create_game_failed; 
	}
	LUNA_GAME = game;

	// Create scene list
	LUNA_DEBUG_LOG("Creating scene list");
	game->_sceneList = _CreateSceneList();
	if (!game->_sceneList) {
		LUNA_ABORT(LUNA_ERROR_STATUS_GENERIC, "Failed to create scene list!");
		goto create_game_failed;
	}

	// Copy settings list
	LUNA_DEBUG_LOG("Copying settings list");
	game->_settingsList = _settingsList;
	if (!game->_settingsList) {
		LUNA_ABORT(LUNA_ERROR_STATUS_GENERIC, "Invalid settings list!");
		goto create_game_failed;
	}

	// Create & load resource list
	LUNA_DEBUG_LOG("Creating resource list");
	game->_resourceDesc = _resourceDesc;
	game->_resourceList = _CreateResourceList(game->_resourceDesc);

	// Create window
	LUNA_DEBUG_LOG("Creating window");
	InitWindow(
		(int)game->_settingsList->_videoSettingsList->_width, 
		(int)game->_settingsList->_videoSettingsList->_height, 
		"GAME"
	);
	SetTargetFPS(60);
	return;

create_game_failed:
	if (game) {
		_DestroySceneList(game->_sceneList);
		_DestroyResourceList(game->_resourceList);
		_DestroyInputSettingsList(game->_settingsList->_inputSettingsList);
	}
	free(game);
	return;
}

void DestroyGame() {
	if (LUNA_GAME) {
		LUNA_DEBUG_LOG("===Destroying game state===");
		_DestroySceneList(LUNA_SCENES);
		_DestroyResourceList(LUNA_RESOURCES);
		_DestroyInputSettingsList(LUNA_INPUTS);
		free(LUNA_GAME);
	}
}

void UpdateGame() {
	LUNA_RETURN_CLEAR;
	if (!LUNA_GAME) { 
		LUNA_DEBUG_WARN("Invalid game state reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	while(!WindowShouldClose()) {
		// Update
		_UpdateInputSettingsList(LUNA_INPUTS);
		_UpdateSceneList(LUNA_SCENES, GetFrameTime());

		// Draw
		BeginDrawing();
		ClearBackground(RAYWHITE);
		_DrawSceneList(LUNA_SCENES);
		EndDrawing();
	}
	CloseWindow();
}

static int settings_ini_handler(void* user, const char* section, const char* name, const char* value) {
	SettingsList* settings = (SettingsList*)user;

	#define INI_MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (INI_MATCH("video", "width")) { 
		settings->_videoSettingsList->_width = atoi(value); 
	} else if (INI_MATCH("video", "height")) { 
		settings->_videoSettingsList->_height = atoi(value); 
	} else if (INI_MATCH("video", "fullscreen")) { 
		settings->_videoSettingsList->_fullscreen = atoi(value); 
	} else {
		return 0;
	}
	return 1;
}

int LoadSettingsFile(char* _settingsFile, SettingsList* _targetSettingsList) {
	LUNA_RETURN_CLEAR;
	int ret = ini_parse(_settingsFile, settings_ini_handler, _targetSettingsList);
#ifdef LUNA_DEBUG
	if (ret == -1) {
		LUNA_DEBUG_WARN("Failed to find settings file (%s)! Loading defaults...", _settingsFile);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
	}
	else if (ret != 0) {
		LUNA_DEBUG_WARN("Unknown value in setting file (%s), line (%d)!", _settingsFile, ret);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_PARAMETER);
	}
#endif
	return ret;
}

SettingsList* GenerateDefaultSettings() {
	LUNA_RETURN_CLEAR;
	InputSlotDesc* inputSlotDesc = NULL;
	SettingsList* settings = NULL;
	
	// Top level settings structure
	settings = calloc(1, sizeof *settings);
	if (!settings) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate settings list!");
		goto generate_default_settings_fail;
	}

	// Video settings
	settings->_videoSettingsList = calloc(1, sizeof *(settings->_videoSettingsList));
	if (!settings->_videoSettingsList) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate video settings list!");
		goto generate_default_settings_fail;
	}
	settings->_videoSettingsList->_width = LUNA_DEFAULT_SETTING_VIDEO_WIDTH;
	settings->_videoSettingsList->_height = LUNA_DEFAULT_SETTING_VIDEO_HEIGHT;
	settings->_videoSettingsList->_fullscreen = LUNA_DEFAULT_SETTING_VIDEO_FULLSCREEN;

	// Input settings
	size_t numInputSlots = 1;
	inputSlotDesc = calloc(numInputSlots, sizeof *inputSlotDesc);
	if (!inputSlotDesc) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate input slot descriptor array!");
		goto generate_default_settings_fail;
	}
	for(size_t i=0; i<numInputSlots; ++i) {
		inputSlotDesc[i]._gamepadDeadzone = LUNA_DEFAULT_SETTING_INPUT_GAMEPAD_DEADZONE;
		inputSlotDesc[i]._gamepadIndex = LUNA_DEFAULT_SETTING_INPUT_GAMEPAD_INDEX;
		inputSlotDesc[i]._mode = LUNA_DEFAULT_SETTING_INPUT_MODE;
		inputSlotDesc[i]._keyboardMapping[INPUT_A] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_A;
		inputSlotDesc[i]._keyboardMapping[INPUT_B] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_B;
		inputSlotDesc[i]._keyboardMapping[INPUT_X] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_X;
		inputSlotDesc[i]._keyboardMapping[INPUT_Y] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_Y;
		inputSlotDesc[i]._keyboardMapping[INPUT_L] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_L;
		inputSlotDesc[i]._keyboardMapping[INPUT_R] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_R;
		inputSlotDesc[i]._keyboardMapping[INPUT_START] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_START;
		inputSlotDesc[i]._keyboardMapping[INPUT_SELECT] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_SELECT;
		inputSlotDesc[i]._keyboardMapping[INPUT_UP] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_UP;
		inputSlotDesc[i]._keyboardMapping[INPUT_DOWN] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_DOWN;
		inputSlotDesc[i]._keyboardMapping[INPUT_LEFT] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_LEFT;
		inputSlotDesc[i]._keyboardMapping[INPUT_RIGHT] = LUNA_DEFAULT_SETTING_INPUT_KEYBOARD_MAPPING_RIGHT;
	}
	settings->_inputSettingsList = _CreateInputSettingsList(inputSlotDesc, numInputSlots);
	if (!settings->_inputSettingsList) {
		LUNA_ABORT(LUNA_ERROR_STATUS_GENERIC, "Failed to create input settings list!");
		goto generate_default_settings_fail;
	}
	free(inputSlotDesc);

	return settings;
generate_default_settings_fail:
	if (settings) {
		free(settings->_videoSettingsList);
		_DestroyInputSettingsList(settings->_inputSettingsList);
	}
	free(settings);
	free(inputSlotDesc);
	return NULL;
}