#include "luna/game.h"

Game* CreateGame(GameDesc _desc) {
	LUNA_DBG_LOG("===Creating game state===");
	Game* game = calloc(1, sizeof *game);
	if (!game) { return NULL; }

	size_t nameLen = strlen(_desc.name) + 1;
	game->width = _desc.width;
	game->height = _desc.height;
	game->name = malloc(nameLen);
	game->sceneList = CreateSceneList();
	if (!game->name || !game->sceneList) {
		free(game);
		return NULL;
	}
	strcpy_s(game->name, nameLen, _desc.name);

	InitWindow((int)game->width, (int)game->height, game->name);
	SetTargetFPS(60);

	return game;
}

void DestroyGame(Game* _game) {
	LUNA_DBG_LOG("===Destroying game state===");
	if (_game) {
		free(_game->name);
		DestroySceneList(_game->sceneList);
		DestroyResourceList(_game->resourceList);
		free(_game);
	}
}

void UpdateGame(Game* _game) {
	if (!_game) { return; }

	while(!WindowShouldClose()) {
		SceneList* list = _game->sceneList;

		// Update
		UpdateSceneList(list, GetFrameTime());

		// Draw
		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawSceneList(list);
		EndDrawing();
	}
	CloseWindow();
}

void LoadResourceFile(Game* _game, const char* _resourceFile, const char* _resourcePassword) {
	// Error check
	if (!_game) { return; }

	// Create new resource list
	ResourceList* list = CreateResourceList(_resourceFile, _resourcePassword);
	if (!list) {
		LUNA_DBG_ERR("Failed to load resource file (%s)", _resourceFile);
		return;
	}
	DestroyResourceList(_game->resourceList);
	_game->resourceList = list;
	return;
}