#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/camera.hpp>
#include <luna/detail/sprite.hpp>
#include <luna/detail/actor.hpp>

namespace luna {

// Forward declarations
class Game;
class Room;
class RoomManager;

struct RoomInit {
	std::function<void(Room*)> pushFunc    = [](Room*) {};
	std::function<void(Room*)> popFunc     = [](Room*) {};
	std::function<void(Room*)> exposedFunc = [](Room*) {};
	std::function<void(Room*)> coveredFunc = [](Room*) {};
	SDL_Color clearColor = LunaColorWhite;
};

class Room {
public:
	LUNA_API Room(RoomInit init);

	LUNA_API SDL_Color GetClearColor() const;

	LUNA_API ActorList* GetActorList();

	LUNA_API std::size_t GetActiveCameraIndex() const;
	LUNA_API std::size_t GetNumCameras() const;
	LUNA_API Camera* GetCamera(std::size_t index);
	LUNA_API const Camera* GetCamera(std::size_t index) const;
	LUNA_API Camera* GetActiveCamera();
	LUNA_API const Camera* GetActiveCamera() const;
	LUNA_API void SetActiveCamera(std::size_t index);
	LUNA_API void DestroyCamera(std::size_t index);

	LUNA_API Camera* CreateCamera();
	LUNA_API Camera* CreateCamera(Camera camera);
	LUNA_API Camera* CreateCamera(Camera&& camera);
	LUNA_API Camera* CreateCamera(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height, std::int32_t zNear, std::int32_t zFar);

protected:
	friend class RoomManager;
	SDL_Color m_clearColor;
	std::vector<Camera> m_cameras = {};
	std::size_t m_activeCameraIndex = 0;
	ActorList m_actors;
	std::function<void(Room*)> m_pushFunc =    [](Room*) {};
	std::function<void(Room*)> m_popFunc =     [](Room*) {};
	std::function<void(Room*)> m_exposedFunc = [](Room*) {};
	std::function<void(Room*)> m_coveredFunc = [](Room*) {};
};

class RoomManager {
public:
	LUNA_API RoomManager() = delete;
	LUNA_API ~RoomManager() = delete;

	LUNA_API static void PushRoom(RoomInit init);
	LUNA_API static void PopRoom();
	LUNA_API static std::size_t RoomStackSize();
	LUNA_API static void GotoRoom(RoomInit init);
	LUNA_API static Room* GetCurrentRoom();

protected:
	friend class Game;
	static void Tick(float dt);
	static void Draw(float dt);

private:
	static std::stack<Room> m_rooms;
};

} // luna