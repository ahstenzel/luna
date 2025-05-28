#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/camera.hpp>
#include <luna/detail/sprite.hpp>

namespace luna {

// Forward declarations
class Game;
class RoomManager;

struct RoomInit {
	std::function<void()> pushFunc;
	std::function<void()> popFunc;
	std::function<void()> exposedFunc;
	std::function<void()> coveredFunc;
};

class Room {
public:
	LUNA_API Room(RoomInit* init);

	LUNA_API SpriteList* GetSpriteList();

protected:
	friend class RoomManager;
	SpriteList m_spriteList;
	std::function<void()> m_pushFunc =    []() {};
	std::function<void()> m_popFunc =     []() {};
	std::function<void()> m_exposedFunc = []() {};
	std::function<void()> m_coveredFunc = []() {};
};

class RoomManager {
public:
	LUNA_API RoomManager() = delete;
	LUNA_API ~RoomManager() = delete;

	LUNA_API static void PushRoom(RoomInit* init);
	LUNA_API static void PopRoom();
	LUNA_API static std::size_t RoomStackSize();
	LUNA_API static void GotoRoom(RoomInit* init);
	LUNA_API static Room* CurrentRoom();

protected:
	friend class Game;
	static void Tick(float dt);

private:
	static std::stack<Room> m_rooms;
};

} // luna