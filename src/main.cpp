#include "game.hpp"
#include <SDL3/SDL_main.h>

int main(int argc, char** argv) {
	if (!Game::Init()) { return 1; }

	auto testFile = ResourceManager::LoadResourceFile("test.arc");
	if (testFile == RESOURCE_FILE_NULL) {
		SDL_Log(ResourceManager::ErrorMessage().c_str());
	}
	else {
		auto textureSmile = ResourceManager::GetTexture(testFile, "smile");
		if (!textureSmile) {
			SDL_Log(ResourceManager::ErrorMessage().c_str());
		}
		else {
			SDL_Log("Texture: (%d, %d)", (int)textureSmile->GetWidth(), (int)textureSmile->GetHeight());
		}
	}
	
	return Game::Run();
}