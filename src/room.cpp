#include <luna/detail/room.hpp>

namespace luna {

std::stack<Room> RoomManager::m_rooms;

Room::Room(RoomInit* init) {
	if (!init) { return; }
	m_pushFunc = init->pushFunc;
	m_popFunc = init->popFunc;
	m_exposedFunc = init->exposedFunc;
	m_coveredFunc = init->coveredFunc;
}

SpriteList* Room::GetSpriteList() {
	return &m_spriteList;
}

void RoomManager::PushRoom(RoomInit* init) {
	if (!m_rooms.empty()) { m_rooms.top().m_coveredFunc(); }
	m_rooms.emplace(init);
	m_rooms.top().m_pushFunc();
}

void RoomManager::PopRoom() {
	if (!m_rooms.empty()) {
		m_rooms.top().m_popFunc();
		m_rooms.pop();
		if (!m_rooms.empty()) { m_rooms.top().m_exposedFunc(); }
	}
}

std::size_t RoomManager::RoomStackSize() {
	return m_rooms.size();
}

void RoomManager::GotoRoom(RoomInit* init) {
	if (!m_rooms.empty()) {
		m_rooms.top().m_popFunc();
		m_rooms.pop();
	}
	m_rooms.emplace(init);
	m_rooms.top().m_pushFunc();
}

Room* RoomManager::CurrentRoom() {
	return (m_rooms.empty()) ? nullptr : &m_rooms.top();
}

void RoomManager::Tick(float dt) {
	if (!m_rooms.empty()) {
		m_rooms.top().m_spriteList.Tick(dt);
	}
}

} // luna