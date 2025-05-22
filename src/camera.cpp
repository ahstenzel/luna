#include <luna/detail/camera.hpp>

namespace luna {

glm::mat4x4 CameraProjectionOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
	return glm::mat4x4(
		2.0f / (right - left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / (zNear - zFar), 0.0f,
		(left + right) / (left - right), (top + bottom) / (bottom - top), (zNear - zFar), 1.0f
	);
}

} // luna