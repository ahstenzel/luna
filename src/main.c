#include "luna/game.h"

int main() {
	GameDesc gameDesc = {
		.width = 640,
		.height = 480,
		.name = "LUNA"
	};
	Game* game = CreateGame(gameDesc);
	UpdateGame(game);
	DestroyGame(game);
	return 0;
}