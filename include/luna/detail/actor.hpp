#pragma once

#include <luna/detail/common.hpp>

namespace luna {

// Forward declarations
class Game;
class RoomManager;

class Actor {
protected:
	friend class Game;
	friend class RoomManager;
	virtual void Tick(float dt) = 0;
	virtual void Draw(float dt) = 0;
};

using ActorList = std::vector<Actor*>;

} // luna