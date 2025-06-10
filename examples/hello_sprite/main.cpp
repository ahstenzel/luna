#include <luna/luna.hpp>

using namespace luna;

class ActorRavioli : public Actor {
public:
	ActorRavioli(float x, float y, std::int32_t depth = 0) {
		// These textures are defined in the *.arc file to have a centered origin
		sprRavioli1 = Sprite(ResourceManager::GetTextureID("ravioli1"), x, y, 0, 0.f, depth, 4.0f, 4.0f);
		sprRavioli2 = Sprite(ResourceManager::GetTextureID("ravioli2"), x + 80.f, y, 0, 0.f, depth, 4.0f, 4.0f);
		sprRavioli3 = Sprite(ResourceManager::GetTextureID("ravioli3"), x + 192.f, y, 0, 0.f, depth, 4.0f, 4.0f);
		sprRavioli4_1 = Sprite(ResourceManager::GetTextureID("ravioli4"), x + 230.f, y, 0, 0.f, depth - 1, 4.0f, 4.0f);
		sprRavioli4_2 = Sprite(ResourceManager::GetTextureID("ravioli4"), x + 154.f, y, 0, 0.f, depth + 1, 4.0f, 4.0f);
	}
protected:
	void Tick(float dt) override {
		static float counter = 0.f;

		sprRavioli1.SetRotation(sprRavioli1.GetRotation() + dt);
		sprRavioli2.SetScaleX(4.f + SDL_sinf(counter * 2.f * SDL_PI_F));
		sprRavioli2.SetScaleY(4.f + SDL_cosf(counter * 2.f * SDL_PI_F));
		sprRavioli3.SetAlpha(0.5f + (0.5f * SDL_cosf(counter * 2.f * SDL_PI_F)));

		counter += dt;
		while (counter > 1.f) { counter -= 1.f; }
	}

	void Draw(float dt) override {
		Game::GetRenderer()->DrawSprite(&sprRavioli1);
		Game::GetRenderer()->DrawSprite(&sprRavioli2);
		Game::GetRenderer()->DrawSprite(&sprRavioli3);
		Game::GetRenderer()->DrawSprite(&sprRavioli4_1);
		Game::GetRenderer()->DrawSprite(&sprRavioli4_2);
	}

	Sprite sprRavioli1;
	Sprite sprRavioli2;
	Sprite sprRavioli3;
	Sprite sprRavioli4_1;
	Sprite sprRavioli4_2;
};

void firstRoomPushFunc(Room* currRoom) {
	currRoom->GetActorList()->AddActor(new ActorRavioli(64.f, 64.f, 10));
}

int main(int argc, char** argv) {
	GameInit init = {};
	init.windowTitle = "Hello Sprite";
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
	RoomInit initFirstRoom = {};
	initFirstRoom.clearColor = { 0, 0, 85, 255 };
	//initFirstRoom.clearColor = LunaColorWhite;
	initFirstRoom.pushFunc = firstRoomPushFunc;
	RoomManager::PushRoom(initFirstRoom);
	RoomManager::GetCurrentRoom()->CreateCamera();

	return Game::Run();
}