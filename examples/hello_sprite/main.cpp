#include <luna/luna.hpp>

using namespace luna;

int main(int argc, char** argv) {
	if (!Game::Init()) { return 1; }

	/*
	auto testFile = ResourceManager::LoadResourceFile("test.arc", "password");
	if (testFile == RESOURCE_ID_NULL) {
		SDL_Log(ResourceManager::ErrorMessage().c_str());
	}
	else {
		auto textureIDSmile = ResourceManager::GetTextureID("smile", testFile);
		if (textureIDSmile == RESOURCE_ID_NULL) {
			SDL_Log(ResourceManager::ErrorMessage().c_str());
		}
		else {
			auto textureSmile = ResourceManager::GetTexture(textureIDSmile, testFile);
			if (textureIDSmile == RESOURCE_ID_NULL) {
				SDL_Log(ResourceManager::ErrorMessage().c_str());
			}
			else {
				SDL_Log("Texture: (%d, %d)", (int)textureSmile->GetWidth(), (int)textureSmile->GetHeight());
			}
		}
	}
	*/
	
	return Game::Run();
}