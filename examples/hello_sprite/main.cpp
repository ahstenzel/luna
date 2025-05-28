#include <luna/luna.hpp>

using namespace luna;

int main(int argc, char** argv) {
	GameInit init = {};
	init.appName = "hello_sprite";
	init.appIdentifier = "com.hello_sprite";
	init.rendererFactory = new SpriteRendererFactory;
	if (!Game::Init(&init)) { return 1; }
	delete init.rendererFactory;

	// Load resources
	if (ResourceManager::LoadResourceFile("hello_sprite.arc") == RESOURCE_ID_NULL) {
		SDL_Log("%s", ResourceManager::ErrorMessage().c_str());
		return 1;
	}

	// Create initial room
	RoomManager::PushRoom({});

	// Create sprite
	ResourceID texRavioli = ResourceManager::GetTextureID("ravioli1");
	if (texRavioli == RESOURCE_ID_NULL) {
		SDL_Log("Failed to find texture (ravioli1)");
		return 1;
	}
	Sprite sprRavioli1(texRavioli, 32.f, 32.f);
	RoomManager::CurrentRoom()->GetSpriteList()->AddSprite(sprRavioli1);

	return Game::Run();
}