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
	Sprite sprRavioli4(ResourceManager::GetTextureID("ravioli4"), 0.f, 0.f, 0, 0, 4.0f, 4.0f);
	Sprite sprRavioli3(ResourceManager::GetTextureID("ravioli3"), 64.f, 0.f, 0, 0, 4.0f, 4.0f);
	Sprite sprRavioli2(ResourceManager::GetTextureID("ravioli2"), 0.f, 64.f, 0, 0, 4.0f, 4.0f);
	Sprite sprRavioli1(ResourceManager::GetTextureID("ravioli1"), 64.f, 64.f, 0, 0, 4.0f, 4.0f);
	//const TexturePage* ravioliTexturePage = sprRavioli1.GetTexturePage();
	//ravioliTexturePage->WriteToFile();
	RoomManager::CurrentRoom()->GetSpriteList()->AddSprite(sprRavioli4);
	RoomManager::CurrentRoom()->GetSpriteList()->AddSprite(sprRavioli3);
	RoomManager::CurrentRoom()->GetSpriteList()->AddSprite(sprRavioli2);
	RoomManager::CurrentRoom()->GetSpriteList()->AddSprite(sprRavioli1);

	return Game::Run();
}