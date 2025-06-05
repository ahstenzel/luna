#include <luna/luna.hpp>

using namespace luna;

class ActorRavioli : public Actor {
public:
	ActorRavioli(float x, float y, std::int32_t depth, int ravioliNum) {
		ResourceID textureID = RESOURCE_ID_NULL;
		switch (ravioliNum) {
		case 1: textureID = ResourceManager::GetTextureID("ravioli1"); break;
		case 2: textureID = ResourceManager::GetTextureID("ravioli2"); break;
		case 3: textureID = ResourceManager::GetTextureID("ravioli3"); break;
		default: textureID = ResourceManager::GetTextureID("ravioli4"); break;
		}
		// These textures are defined in the *.arc file to have a centered origin
		sprRavioli = Sprite(textureID, x, y, 0, 0.f, depth, 4.0f, 4.0f);
	}
protected:
	void Tick(float dt) override {
		sprRavioli.SetRotation(sprRavioli.GetRotation() + dt);
	}

	void Draw(float dt) override {
		Game::GetRenderer()->DrawSprite(&sprRavioli);
	}

	Sprite sprRavioli;
};

void firstRoomPushFunc(Room* currRoom) {
	currRoom->GetActorList()->AddActor(new ActorRavioli(32.f, 32.f, 4, 4));
	currRoom->GetActorList()->AddActor(new ActorRavioli(64.f, 32.f, 3, 3));
	currRoom->GetActorList()->AddActor(new ActorRavioli(96.f, 32.f, -10, 2));
	currRoom->GetActorList()->AddActor(new ActorRavioli(128.f, 32.f, 1, 1));
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
	initFirstRoom.clearColor = { 0, 0, 85, 0 };
	initFirstRoom.pushFunc = firstRoomPushFunc;
	RoomManager::PushRoom(initFirstRoom);
	RoomManager::GetCurrentRoom()->CreateCamera();

	return Game::Run();
}