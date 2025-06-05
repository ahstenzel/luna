#include <luna/detail/room.hpp>
#include <luna/detail/game.hpp>
#include <luna/detail/actor.hpp>

namespace luna {

std::stack<Room> RoomManager::m_rooms;

Room::Room(RoomInit init) :
	m_clearColor(init.clearColor),
	m_pushFunc(init.pushFunc),
	m_popFunc(init.popFunc), 
	m_exposedFunc(init.exposedFunc), 
	m_coveredFunc(init.coveredFunc) {}

SDL_Color Room::GetClearColor() const {
	return m_clearColor;
}

ActorList* Room::GetActorList() {
	return &m_actors;
}

std::size_t Room::GetActiveCameraIndex() const {
	return m_activeCameraIndex;
}

std::size_t Room::GetNumCameras() const {
	return m_cameras.size();
}

Camera* Room::GetCamera(std::size_t index) {
	return (index < m_cameras.size()) ? &m_cameras[index] : nullptr;
}

const Camera* Room::GetCamera(std::size_t index) const {
	return (index < m_cameras.size()) ? &m_cameras[index] : nullptr;
}

Camera* Room::GetActiveCamera() {
	return (m_activeCameraIndex < m_cameras.size()) ? &m_cameras[m_activeCameraIndex] : nullptr;
}

const Camera* Room::GetActiveCamera() const {
	return (m_activeCameraIndex < m_cameras.size()) ? &m_cameras[m_activeCameraIndex] : nullptr;
}

void Room::SetActiveCamera(std::size_t index) {
	m_activeCameraIndex = index;
}

void Room::DestroyCamera(std::size_t index) {
	if (index == m_activeCameraIndex) { m_activeCameraIndex = std::clamp(m_activeCameraIndex, std::size_t(0), m_cameras.size() - 1); }
	m_cameras.erase(m_cameras.begin() + index);
}

Camera* Room::CreateCamera() {
	return CreateCamera(0, 0, Game::GetWindowWidth(), Game::GetWindowHeight(), -1000000, 1000000);
}

Camera* Room::CreateCamera(Camera camera) {
	if (m_cameras.size() == 0) { m_activeCameraIndex = 0; }
	m_cameras.push_back(camera);
	return &m_cameras.back();
}

Camera* Room::CreateCamera(Camera&& camera) {
	if (m_cameras.size() == 0) { m_activeCameraIndex = 0; }
	m_cameras.push_back(std::move(camera));
	return &m_cameras.back();
}

Camera* Room::CreateCamera(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height, std::int32_t zNear, std::int32_t zFar) {
	if (m_cameras.size() == 0) { m_activeCameraIndex = 0; }
	m_cameras.emplace_back(x, y, width, height, zNear, zFar);
	return &m_cameras.back();
}

void RoomManager::PushRoom(RoomInit init) {
	if (!m_rooms.empty()) { m_rooms.top().m_coveredFunc(&m_rooms.top()); }
	m_rooms.emplace(init);
	m_rooms.top().m_pushFunc(&m_rooms.top());
}

void RoomManager::PopRoom() {
	if (!m_rooms.empty()) {
		m_rooms.top().m_popFunc(&m_rooms.top());
		m_rooms.pop();
		if (!m_rooms.empty()) { m_rooms.top().m_exposedFunc(&m_rooms.top()); }
	}
}

std::size_t RoomManager::RoomStackSize() {
	return m_rooms.size();
}

void RoomManager::GotoRoom(RoomInit init) {
	if (!m_rooms.empty()) {
		m_rooms.top().m_popFunc(&m_rooms.top());
		m_rooms.pop();
	}
	m_rooms.emplace(init);
	m_rooms.top().m_pushFunc(&m_rooms.top());
}

Room* RoomManager::GetCurrentRoom() {
	return (m_rooms.empty()) ? nullptr : &m_rooms.top();
}

void RoomManager::Tick(float dt) {
	if (m_rooms.empty()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "RoomManager::Tick failed: Room stack is empty!");
		return;
	}
	for (auto& actor : *(m_rooms.top().GetActorList())) {
		actor->Tick(dt);
	}
}

void RoomManager::Draw(float dt) {
	if (m_rooms.empty()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "RoomManager::Draw failed: Room stack is empty!");
		return;
	}
	for (auto& actor : *(m_rooms.top().GetActorList())) {
		actor->Draw(dt);
	}
}

void RoomManager::Clear() {
	while (RoomStackSize() > 0) { PopRoom(); }
}

} // luna