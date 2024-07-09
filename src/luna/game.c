#include "luna/game.h"

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
	game->sceneList = _CreateSceneList();
	if (!game->sceneList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Failed to create scene list!");
		goto create_game_failed;
	}

	// Copy settings list
	LUNA_DBG_LOG("Copying settings list");
	game->settingsList = _settingsList;
	if (!game->settingsList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Invalid settings list!");
		goto create_game_failed;
	}

	// Copy resource list descriptors
	LUNA_DBG_LOG("Copying resource list descriptors");
	game->resourceDesc = _resourceDesc;
	game->numResourceDesc = _numResourceDesc;
	if (!game->resourceDesc) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "NULL resource list descriptors!");
		goto create_game_failed;
	}
	if (game->numResourceDesc == 0) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "No resource list descriptors provided!");
		goto create_game_failed;
	}

	// Create resource list
	LUNA_DBG_LOG("Creating resource list");
	game->resourceList = _CreateResourceList(game->resourceDesc[0]);
	if (!game->resourceList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to create resource list!");
		goto create_game_failed;
	}

	// Create window
	LUNA_DBG_LOG("Creating window");
	InitWindow((int)game->settingsList->videoSettingsList->width, (int)game->settingsList->videoSettingsList->height, "GAME");
	SetTargetFPS(60);
	LUNA_GAME = game;
	return;

create_game_failed:
	if (game) {
		_DestroySceneList(game->sceneList);
		_DestroyResourceList(game->resourceList);
		_DestroyInputSettingsList(game->settingsList->inputSettingsList);
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
	for(size_t i=0; i<LUNA_GAME->numResourceDesc; ++i) {
		if (strcmp(_name, LUNA_GAME->resourceDesc[i].resourceName)) {
			_LoadResourceFile(LUNA_GAME->resourceDesc[i]);
			return;
		}
	}
	LUNA_DBG_WARN("Failed to find a resource list with name (%s)", _name);
}

void LoadResourceFileByIndex(size_t _idx) {
	// Error check
	if (!LUNA_GAME) { return; }

	if (_idx < LUNA_GAME->numResourceDesc) {
		_LoadResourceFile(LUNA_GAME->resourceDesc[_idx]);
	}
	LUNA_DBG_WARN("Resource list %d does not exist, only %d descriptors are registered", (int)_idx, (int)LUNA_GAME->numResourceDesc);
}

void _LoadResourceFile(ResourceListDesc _desc) {
	// Error check
	if (!LUNA_GAME) { return; }

	// Create new resource list
	ResourceList* list = _CreateResourceList(_desc);
	if (!list) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_FILESYS, "Failed to load resource file (%s)", _desc.resourceFile);
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
		settings->videoSettingsList->width = atoi(value); 
	} else if (INI_MATCH("video", "height")) { 
		settings->videoSettingsList->height = atoi(value); 
	} else if (INI_MATCH("video", "fullscreen")) { 
		settings->videoSettingsList->fullscreen = atoi(value); 
	} else {
		return 0;
	}
	return 1;
}

int LoadSettingsFile(char* _settingsFile, SettingsList* _targetSettingsList) {
	int ret = ini_parse(_settingsFile, settings_ini_handler, _targetSettingsList);
#ifdef LUNA_DEBUG
	if (ret == -1) {
		LUNA_DBG_WARN("Failed to find settings file (%s), loading defaults", _settingsFile);
	}
	else if (ret != 0) {
		LUNA_DBG_WARN("Unknown value in setting file (%s), line (%d)", _settingsFile, ret);
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
		goto generate_settings_fail;
	}

	// Video settings
	settings->videoSettingsList = calloc(1, sizeof *(settings->videoSettingsList));
	if (!settings->videoSettingsList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate video settings list!");
		goto generate_settings_fail;
	}
	settings->videoSettingsList->width = 640;
	settings->videoSettingsList->height = 480;
	settings->videoSettingsList->fullscreen = false;

	// Input settings
	size_t numInputSlots = 1;
	inputSlotDesc = calloc(numInputSlots, sizeof *inputSlotDesc);
	if (!inputSlotDesc) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate input slot descriptor array!");
		goto generate_settings_fail;
	}
	for(size_t i=0; i<numInputSlots; ++i) {
		inputSlotDesc[i].gamepadDeadzone = 0.1f;
		inputSlotDesc[i].gamepadIndex = -1;
		inputSlotDesc[i].mode = INPUT_MODE_KEYBOARD;
		inputSlotDesc[i].keyboardMapping[INPUT_A] = KEY_Z;
		inputSlotDesc[i].keyboardMapping[INPUT_B] = KEY_X;
		inputSlotDesc[i].keyboardMapping[INPUT_X] = KEY_A;
		inputSlotDesc[i].keyboardMapping[INPUT_Y] = KEY_S;
		inputSlotDesc[i].keyboardMapping[INPUT_L] = KEY_LEFT_SHIFT;
		inputSlotDesc[i].keyboardMapping[INPUT_R] = KEY_RIGHT_SHIFT;
		inputSlotDesc[i].keyboardMapping[INPUT_START] = KEY_ENTER;
		inputSlotDesc[i].keyboardMapping[INPUT_SELECT] = KEY_TAB;
		inputSlotDesc[i].keyboardMapping[INPUT_UP] = KEY_UP;
		inputSlotDesc[i].keyboardMapping[INPUT_DOWN] = KEY_DOWN;
		inputSlotDesc[i].keyboardMapping[INPUT_LEFT] = KEY_LEFT;
		inputSlotDesc[i].keyboardMapping[INPUT_RIGHT] = KEY_RIGHT;
	}
	settings->inputSettingsList = _CreateInputSettingsList(inputSlotDesc, numInputSlots);
	if (!settings->inputSettingsList) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_GENERIC, "Failed to create input settings list!");
		goto generate_settings_fail;
	}
	free(inputSlotDesc);

	return settings;
generate_settings_fail:
	if (settings) {
		free(settings->videoSettingsList);
		_DestroyInputSettingsList(settings->inputSettingsList);
	}
	free(settings);
	free(inputSlotDesc);
	return NULL;
}