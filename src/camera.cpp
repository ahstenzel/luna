#include <luna/detail/camera.hpp>

namespace luna {

Camera::Camera(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height, std::int32_t zNear, std::int32_t zFar) :
	m_cameraX(x),
	m_cameraY(y),
	m_cameraW(width),
	m_cameraH(height),
	m_zNear(zNear),
	m_zFar(zFar) {}

std::int32_t Camera::GetPositionX() const {
	return m_cameraX;
}

std::int32_t Camera::GetPositionY() const {
	return m_cameraY;
}

std::uint32_t Camera::GetWidth() const {
	return m_cameraW;
}

std::uint32_t Camera::GetHeight() const {
	return m_cameraH;
}

std::int32_t Camera::GetNearPlane() const {
	return m_zNear;
}

std::int32_t Camera::GetFarPlane() const {
	return m_zFar;
}

std::int32_t Camera::GetLeftEdge() const {
	return m_cameraX;
}

std::int32_t Camera::GetRightEdge() const {
	return m_cameraX + std::int32_t(m_cameraW);
}

std::int32_t Camera::GetTopEdge() const {
	return m_cameraY;
}

std::int32_t Camera::GetBottomEdge() const {
	return m_cameraY + std::int32_t(m_cameraH);
}

bool Camera::PointOnCamera(float x, float y) const {
	return (
		x >= float(m_cameraX) && x < (float(m_cameraX) + float(m_cameraW)) &&
		y >= float(m_cameraY) && y < (float(m_cameraY) + float(m_cameraH))
	);
}

int Camera::RegionOnCamera(float left, float right, float top, float bottom) const {
	int v = 0;
	if (PointOnCamera(left, top)) { v++; }
	if (PointOnCamera(right, top)) { v++; }
	if (PointOnCamera(left, bottom)) { v++; }
	if (PointOnCamera(right, bottom)) { v++; }
	if (v == 0) { return 0; }
	else if (v < 4) { return 1; }
	else { return 2; }
}

void Camera::SetPositionX(std::int32_t x) {
	m_cameraX = x;
}

void Camera::SetPositionY(std::int32_t y) {
	m_cameraY = y;
}

void Camera::SetWidth(std::uint32_t width) {
	m_cameraW = width;
}

void Camera::SetHeight(std::uint32_t height) {
	m_cameraH = height;
}

void Camera::SetNearPlane(std::int32_t zNear) {
	m_zNear = zNear;
}

void Camera::SetFarPlane(std::int32_t zFar) {
	m_zFar = zFar;
}

glm::mat4 Camera::ProjectionOrtho() const {
	float left = (float)m_cameraX;
	float right = (float)(m_cameraX + std::int32_t(m_cameraW));
	float top = (float)m_cameraY;
	float bottom = (float)(m_cameraY + std::int32_t(m_cameraH));
	float zNear = (float)(m_zNear) - 1.0f;
	float zFar = (float)(m_zFar) + 1.0f;
	return glm::mat4(
		{ 2.0f / (right - left), 0.f, 0.f, 0.f },
		{ 0.f, 2.0f / (top - bottom), 0.f, 0.f },
		{ 0.f, 0.f, -2.0f / (zFar - zNear), 0.f },
		{ -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), 1.f }
	);
}

} // luna