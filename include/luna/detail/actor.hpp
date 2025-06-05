#pragma once

#include <luna/detail/common.hpp>

namespace luna {

// Forward declarations
class Game;
class RoomManager;
class ActorList;

class Actor {
public:
	bool HasTag(const std::string& tag);
	bool HasTagsAny(const std::unordered_set<std::string>& tags);
	bool HasTagsAll(const std::unordered_set<std::string>& tags);
	void AddTag(const std::string& tag);
	void RemoveTag(const std::string& tag);
protected:
	friend class Game;
	friend class RoomManager;
	LUNA_API virtual void Tick(float dt) = 0;
	LUNA_API virtual void Draw(float dt) = 0;

	std::unordered_set<std::string> m_tags;
};

//using ActorList = std::vector<Actor*>;

class ActorList {
public:
	ActorList();
	ActorList(const ActorList& other);
	ActorList(ActorList&& other) noexcept;
	~ActorList();

	using ActorItr = std::vector<Actor*>::iterator;

	LUNA_API ActorItr AddActor(Actor* actor);
	LUNA_API void RemoveActor(ActorItr it);
	LUNA_API void QueueRemoveActor(ActorItr it);
	LUNA_API std::vector<ActorItr> FindActorsAny(const std::unordered_set<std::string>& tags);
	LUNA_API std::vector<ActorItr> FindActorsAll(const std::unordered_set<std::string>& tags);
	LUNA_API void Clear();
	LUNA_API std::size_t Size();
	
	LUNA_API ActorItr begin();
	LUNA_API ActorItr end();
protected:
	friend class Game;
	friend class RoomManager;

	void FlushActors();
private:
	std::vector<Actor*> m_actors;
	std::vector<unsigned char> m_flaggedActors;
};

} // luna