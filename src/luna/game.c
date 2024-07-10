#include "luna/game.h"
#include "luna/defaults.h"

Game* _luna_game_instance = NULL;

void CreateGame(SettingsList* _settingsList, ResourceListDesc* _resourceDesc, size_t _numResourceDesc) {
	LUNA_DBG_LOG("===Creating game state===");
	Game* game = calloc(1, sizeof *game);
	if (!game) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate game state!");
		goto create_game_failed; 
	}

	// Create scene list
	LUNA_DBG_LOG("Creating scene list");
	game->_sceneList = _CreateSceneList();
	if (!game->_sceneList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Failed to create scene list!");
		goto create_game_failed;
	}

	// Copy settings list
	LUNA_DBG_LOG("Copying settings list");
	game->_settingsList = _settingsList;
	if (!game->_settingsList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Invalid settings list!");
		goto create_game_failed;
	}

	// Copy resource list descriptors
	LUNA_DBG_LOG("Copying resource list descriptors");
	game->_resourceDescList = vector_create(ResourceListDesc);
	if (!game->_resourceDescList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Failed to allocate resource file descriptor list!");
		goto create_game_failed;
	}
	if (_numResourceDesc == 0 || !_resourceDesc) {
		LUNA_DBG_WARN("No resource list descriptors provided!");
	}
	else {
		for(size_t i=0; i<_numResourceDesc; ++i) {
			vector_push_back(game->_resourceDescList, &_resourceDesc[i]);
		}
	}

	// Create resource list
	LUNA_DBG_LOG("Creating resource list");
	if (vector_size(game->_resourceDescList) > 0) {
		ResourceListDesc* resourceListDesc = vector_get(game->_resourceDescList, 0);
		if (resourceListDesc) {
			game->_resourceList = _CreateResourceList(*resourceListDesc);
			if (!game->_resourceList) {
				LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to create resource list!");
				goto create_game_failed;
			}
		}
		else {
			LUNA_DBG_WARN("Invalid resource file descriptor (0)!");
		}
	}

	// Create window
	LUNA_DBG_LOG("Creating window");
	InitWindow(
		(int)game->_settingsList->_videoSettingsList->_width, 
		(int)game->_settingsList->_videoSettingsList->_height, 
		"GAME"
	);
	SetTargetFPS(60);
	LUNA_GAME = game;
	return;

create_game_failed:
	if (game) {
		_DestroySceneList(game->_sceneList);
		_DestroyResourceList(game->_resourceList);
		_DestroyInputSettingsList(game->_settingsList->_inputSettingsList);
		vector_destroy(game->_resourceDescList);
	}
	free(game);
	return;
}

void DestroyGame() {
	if (LUNA_GAME) {
		LUNA_DBG_LOG("===Destroying game state===");
		_DestroySceneList(LUNA_SCENES);
		_DestroyResourceList(LUNA_RESOURCES);
		_DestroyInputSettingsList(LUNA_INPUTS);
		vector_destroy(LUNA_GAME->_resourceDescList);
		free(LUNA_GAME);
	}
}

void UpdateGame() {
	if (!LUNA_GAME) { return; }

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

void LoadResourceFileByName(const char* _name) {
	// Error check
	if (!LUNA_GAME) { return; }

	// Search list for matching name
	for(size_t i=0; i<vector_size(LUNA_GAME->_resourceDescList); ++i) {
		ResourceListDesc* resourceListDesc = vector_get(LUNA_GAME->_resourceDescList, i);
		if (resourceListDesc) {
			if (strcmp(_name, resourceListDesc->resourceName)) {
				_LoadResourceFile(*resourceListDesc);
				return;
			}
			else {
				LUNA_DBG_WARN("Failed to find a resource list with name (%s)", _name);
			}
		}
		else {
			LUNA_DBG_WARN("Invalid resource file descriptor (%d)!", (int)i);
		}
	}
}

void LoadResourceFileByIndex(size_t _idx) {
	// Error check
	if (!LUNA_GAME) { return; }

	if (_idx < vector_size(LUNA_GAME->_resourceDescList)) {
		ResourceListDesc* resourceListDesc = vector_get(LUNA_GAME->_resourceDescList, _idx);
		if (resourceListDesc) {
			_LoadResourceFile(*resourceListDesc);
		}
		else {
			LUNA_DBG_WARN("Invalid resource file descriptor (%d)!", (int)_idx);
		}
	}
	LUNA_DBG_WARN("Resource list (%d) does not exist, only (%d) descriptors are registered", 
		(int)_idx, 
		(int)vector_size(LUNA_GAME->_resourceDescList)
	);
}

void _LoadResourceFile(ResourceListDesc _desc) {
	// Error check
	if (!LUNA_GAME) { return; }

	// Create new resource list
	ResourceList* list = _CreateResourceList(_desc);
	if (!list) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_FILESYS, "Failed to load resource file (%s)!", _desc.resourceFile);
		return;
	}
	_DestroyResourceList(LUNA_RESOURCES);
	LUNA_RESOURCES = list;
	return;
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
	int ret = ini_parse(_settingsFile, settings_ini_handler, _targetSettingsList);
#ifdef LUNA_DEBUG
	if (ret == -1) {
		LUNA_DBG_WARN("Failed to find settings file (%s)! Loading defaults...", _settingsFile);
	}
	else if (ret != 0) {
		LUNA_DBG_WARN("Unknown value in setting file (%s), line (%d)!", _settingsFile, ret);
	}
#endif
	return ret;
}

SettingsList* GenerateDefaultSettings() {
	InputSlotDesc* inputSlotDesc = NULL;
	SettingsList* settings = NULL;
	
	// Top level settings structure
	settings = calloc(1, sizeof *settings);
	if (!settings) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate settings list!");
		goto generate_default_settings_fail;
	}

	// Video settings
	settings->_videoSettingsList = calloc(1, sizeof *(settings->_videoSettingsList));
	if (!settings->_videoSettingsList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate video settings list!");
		goto generate_default_settings_fail;
	}
	settings->_videoSettingsList->_width = LUNA_DEFAULT_SETTING_VIDEO_WIDTH;
	settings->_videoSettingsList->_height = LUNA_DEFAULT_SETTING_VIDEO_HEIGHT;
	settings->_videoSettingsList->_fullscreen = LUNA_DEFAULT_SETTING_VIDEO_FULLSCREEN;

	// Input settings
	size_t numInputSlots = 1;
	inputSlotDesc = calloc(numInputSlots, sizeof *inputSlotDesc);
	if (!inputSlotDesc) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate input slot descriptor array!");
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
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Failed to create input settings list!");
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