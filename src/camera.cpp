#include <luna/detail/camera.hpp>

namespace luna {

glm::mat4 CameraProjectionOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
	return glm::mat4(
		{ 2.0f / (right - left), 0.f, 0.f, 0.f },
		{ 0.f, 2.0f / (top - bottom), 0.f, 0.f },
		{ 0.f, 0.f, -2.0f / (zFar - zNear), 0.f },
		{ -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zFar + zNear) / (zFar - zNear), 1.f }
	);
}

} // luna