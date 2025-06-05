#include <luna/detail/actor.hpp>

namespace luna {

bool Actor::HasTag(const std::string& tag) {
	return m_tags.count(tag) > 0;
}

bool Actor::HasTagsAny(const std::unordered_set<std::string>& tags) {
	bool found = false;
	for (auto& tag : tags) {
		if (m_tags.count(tag) > 0) {
			found = true;
			break;
		}
	}
	return found;
}

bool Actor::HasTagsAll(const std::unordered_set<std::string>& tags) {
	bool found = true;
	for (auto& tag : tags) {
		if (m_tags.count(tag) == 0) {
			found = false;
			break;
		}
	}
	return found;
}

void Actor::AddTag(const std::string& tag) {
	m_tags.insert(tag);
}

void Actor::RemoveTag(const std::string& tag) {
	m_tags.erase(tag);
}

ActorList::ActorList() = default;

ActorList::ActorList(const ActorList& other) :
	m_actors(other.m_actors),
	m_flaggedActors(other.m_flaggedActors) {}

ActorList::ActorList(ActorList&& other) noexcept :
	m_actors(std::move(other.m_actors)),
	m_flaggedActors(std::move(other.m_flaggedActors)) {}

ActorList::~ActorList() {
	Clear();
}

ActorList::ActorItr ActorList::AddActor(Actor* actor) {
	m_actors.push_back(actor);
	m_flaggedActors.push_back(0);
	return m_actors.end() - 1;
}

void ActorList::RemoveActor(ActorItr it) {
	std::size_t idx = it - m_actors.begin();
	if (idx >= m_actors.size()) { return; }
	delete (*it);
	m_actors.erase(it);
	m_flaggedActors.erase(m_flaggedActors.begin() + idx);
}

void ActorList::QueueRemoveActor(ActorItr it) {
	std::size_t idx = it - m_actors.begin();
	if (idx >= m_actors.size()) { return; }
	m_flaggedActors[idx] |= 0x01;
}

std::vector<ActorList::ActorItr> ActorList::FindActorsAny(const std::unordered_set<std::string>& tags) {
	std::vector<ActorItr> foundActors;
	for (ActorItr it = begin(); it != end(); ++it) {
		if ((*it)->HasTagsAny(tags)) {
			foundActors.push_back(it);
		}
	}
	return foundActors;
}

std::vector<ActorList::ActorItr> ActorList::FindActorsAll(const std::unordered_set<std::string>& tags) {
	std::vector<ActorItr> foundActors;
	for (ActorItr it = begin(); it != end(); ++it) {
		if ((*it)->HasTagsAll(tags)) {
			foundActors.push_back(it);
		}
	}
	return foundActors;
}

void ActorList::Clear() {
	for (auto& actor : m_actors) {
		delete actor;
	}
	m_actors.clear();
	m_flaggedActors.clear();
}

std::size_t ActorList::Size() {
	return m_actors.size();
}

ActorList::ActorItr ActorList::begin() {
	return m_actors.begin();
}

ActorList::ActorItr ActorList::end() {
	return m_actors.end();
}

void ActorList::FlushActors() {
	for (int i = 0; i < int(m_actors.size()); ++i) {
		if (m_flaggedActors[i] &= 0x01) {
			delete m_actors[i];
			m_actors.erase(m_actors.begin() + i);
			m_flaggedActors.erase(m_flaggedActors.begin() + i);
			i--;
		}
	}
}

} // luna