#include "game.hpp"
#include <SDL3/SDL_main.h>

int main(int argc, char** argv) {
	if (!Game::Init()) { return 1; }
	return Game::Run();
}