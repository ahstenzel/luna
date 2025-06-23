#include <luna/detail/camera.hpp>

namespace luna {

Camera::Camera(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height, std::int32_t zNear, std::int32_t zFar) :
	m_cameraX(x),
	m_cameraY(y),
	m_cameraW(width),
	m_cameraH(height),
	m_zNear(zNear),
	m_zFar(zFar) {
	CalculateBoundingBox();
}

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
	return PointInShape(x, y, m_bbox);
}

bool Camera::RegionOnCamera(const ShapeLine& shape) const {
	return ShapeIntersects(shape, m_bbox);
}

bool Camera::RegionOnCamera(const ShapeAABB& shape) const {
	return ShapeIntersects(shape, m_bbox);
}

bool Camera::RegionOnCamera(const ShapeCircle& shape) const {
	return ShapeIntersects(shape, m_bbox);
}

bool Camera::RegionOnCamera(const AnyShape& shape, ShapeType type) const {
	switch (type) {
	case ShapeType::LineType: return ShapeIntersects(std::get<ShapeLine>(shape), m_bbox); break;
	case ShapeType::AABBType: return ShapeIntersects(std::get<ShapeAABB>(shape), m_bbox); break;
	case ShapeType::CircleType: return ShapeIntersects(std::get<ShapeCircle>(shape), m_bbox); break;
	}
	return false;
}

void Camera::SetPositionX(std::int32_t x) {
	m_cameraX = x;
	CalculateBoundingBox();
}

void Camera::SetPositionY(std::int32_t y) {
	m_cameraY = y;
	CalculateBoundingBox();
}

void Camera::SetWidth(std::uint32_t width) {
	m_cameraW = width;
	CalculateBoundingBox();
}

void Camera::SetHeight(std::uint32_t height) {
	m_cameraH = height;
	CalculateBoundingBox();
}

void Camera::SetNearPlane(std::int32_t zNear) {
	m_zNear = zNear;
}

void Camera::SetFarPlane(std::int32_t zFar) {
	m_zFar = zFar;
}

glm::mat4 Camera::ProjectionOrtho() const {
	//float left = (float)m_cameraX;
	//float right = (float)(m_cameraX + std::int32_t(m_cameraW));
	//float top = (float)m_cameraY;
	//float bottom = (float)(m_cameraY + std::int32_t(m_cameraH));
	float zNear = (float)(m_zNear) - 1.0f;
	float zFar = (float)(m_zFar) + 1.0f;
	return glm::mat4(
		{ 2.0f / (m_bbox.right - m_bbox.left), 0.f, 0.f, 0.f },
		{ 0.f, 2.0f / (m_bbox.top - m_bbox.bottom), 0.f, 0.f },
		{ 0.f, 0.f, -2.0f / (zFar - zNear), 0.f },
		{ -(m_bbox.right + m_bbox.left) / (m_bbox.right - m_bbox.left), -(m_bbox.top + m_bbox.bottom) / (m_bbox.top - m_bbox.bottom), -(zFar + zNear) / (zFar - zNear), 1.f }
	);
}

void Camera::CalculateBoundingBox() {
	m_bbox.left = float(m_cameraX);
	m_bbox.top = float(m_cameraY);
	m_bbox.right = float(m_cameraX + std::int32_t(m_cameraW));
	m_bbox.bottom = float(m_cameraY + std::int32_t(m_cameraH));
}

} // luna