#pragma once

#include <luna/detail/common.hpp>

namespace luna {

// Forward declarations
class Game;
class RoomManager;
class ActorList;

class Actor {
public:
	LUNA_API bool HasTag(const std::string& tag);
	LUNA_API bool HasTagsAny(const std::unordered_set<std::string>& tags);
	LUNA_API bool HasTagsAll(const std::unordered_set<std::string>& tags);
	LUNA_API void AddTag(const std::string& tag);
	LUNA_API void RemoveTag(const std::string& tag);

	LUNA_API bool GetVisible() const;
	LUNA_API void SetVisible(bool visible);
	LUNA_API bool GetActive() const;
	LUNA_API void SetActive(bool active);
protected:
	friend class Game;
	friend class RoomManager;
	virtual void Tick(float dt) = 0;
	virtual void Draw(float dt) = 0;

	std::unordered_set<std::string> m_tags;
	bool m_active = true;
	bool m_visible = true;
};

class ActorList {
public:
	LUNA_API ActorList();
	LUNA_API ActorList(const ActorList& other);
	LUNA_API ActorList(ActorList&& other) noexcept;
	LUNA_API ~ActorList();

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