#include "luna/game.h"

Game* _luna_game_instance = NULL;

void CreateGame(GameDesc _gameDesc, ResourceListDesc _resourceDesc, InputDesc* _inputDesc, size_t _inputNum) {
	LUNA_DBG_LOG("===Creating game state===");
	Game* game = calloc(1, sizeof *game);
	if (!game) { return; }

	size_t nameLen = strlen(_gameDesc.name) + 1;
	game->width = _gameDesc.width;
	game->height = _gameDesc.height;
	game->name = malloc(nameLen);
	game->sceneList = CreateSceneList();
	game->resourceList = CreateResourceList(_resourceDesc);
	game->inputList = CreateInputList(_inputDesc, _inputNum);
	if (!game->name || !game->sceneList || !game->resourceList || !game->inputList) {
#ifdef LUNA_DEBUG
		if (!game->name)         { LUNA_DBG_LOG("ERROR (CreateGame): Failed to allocate game name!"); }
		if (!game->sceneList)    { LUNA_DBG_LOG("ERROR (CreateGame): Failed to create scene list!"); }
		if (!game->resourceList) { LUNA_DBG_LOG("ERROR (CreateGame): Failed to create resource list!"); }
		if (!game->inputList)    { LUNA_DBG_LOG("ERROR (CreateGame): Failed to create input list!"); }
#endif
		DestroyGame();
		return;
	}
	strcpy_s(game->name, nameLen, _gameDesc.name);

	InitWindow((int)game->width, (int)game->height, game->name);
	SetTargetFPS(60);

	LUNA_GAME = game;
}

void DestroyGame() {
	if (LUNA_GAME) {
		LUNA_DBG_LOG("===Destroying game state===");
		free(LUNA_GAME->name);
		DestroySceneList(LUNA_SCENES);
		DestroyResourceList(LUNA_RESOURCES);
		DestroyInputList(LUNA_INPUTS);
		free(LUNA_GAME);
	}
}

void UpdateGame() {
	if (!LUNA_GAME) { return; }

	while(!WindowShouldClose()) {
		// Update
		UpdateInputList(LUNA_INPUTS);
		UpdateSceneList(LUNA_SCENES, GetFrameTime());

		// Draw
		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawSceneList(LUNA_SCENES);
		EndDrawing();
	}
	CloseWindow();
}

void LoadResourceFile(ResourceListDesc _desc) {
	// Error check
	if (!LUNA_GAME) { return; }

	// Create new resource list
	ResourceList* list = CreateResourceList(_desc);
	if (!list) {
		LUNA_DBG_ERR("Failed to load resource file (%s)", _desc.resourceFile);
		return;
	}
	DestroyResourceList(LUNA_RESOURCES);
	LUNA_RESOURCES = list;
	return;
}