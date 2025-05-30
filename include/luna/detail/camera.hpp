#pragma once

#include <luna/detail/common.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace luna {

LUNA_API glm::mat4 CameraProjectionOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

} // luna