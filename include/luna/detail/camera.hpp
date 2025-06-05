#pragma once

#include <luna/detail/common.hpp>

namespace luna {

class Camera {
public:
	Camera(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height, std::int32_t zNear, std::int32_t zFar);

	LUNA_API std::int32_t GetPositionX() const;
	LUNA_API std::int32_t GetPositionY() const;
	LUNA_API std::uint32_t GetWidth() const;
	LUNA_API std::uint32_t GetHeight() const;
	LUNA_API std::int32_t GetNearPlane() const;
	LUNA_API std::int32_t GetFarPlane() const;
	LUNA_API std::int32_t GetLeftEdge() const;
	LUNA_API std::int32_t GetRightEdge() const;
	LUNA_API std::int32_t GetTopEdge() const;
	LUNA_API std::int32_t GetBottomEdge() const;
	LUNA_API bool PointOnCamera(float x, float y) const;
	LUNA_API int RegionOnCamera(float left, float right, float top, float bottom) const;

	LUNA_API void SetPositionX(std::int32_t x);
	LUNA_API void SetPositionY(std::int32_t y);
	LUNA_API void SetWidth(std::uint32_t width);
	LUNA_API void SetHeight(std::uint32_t height);
	LUNA_API void SetNearPlane(std::int32_t zNear);
	LUNA_API void SetFarPlane(std::int32_t zFar);

	LUNA_API glm::mat4 ProjectionOrtho() const;
private:
	std::uint32_t m_viewportW = 0;
	std::uint32_t m_viewportH = 0;
	std::uint32_t m_viewportX = 0;
	std::uint32_t m_viewportY = 0;
	std::int32_t m_cameraW = 0;
	std::int32_t m_cameraH = 0;
	std::int32_t m_cameraX = 0;
	std::int32_t m_cameraY = 0;
	std::int32_t m_zNear = 0;
	std::int32_t m_zFar = 0;
};

} // luna