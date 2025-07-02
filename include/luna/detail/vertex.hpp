#pragma once

#include <luna/detail/common.hpp>

namespace luna {

class VertexPos {
public:
	LUNA_API VertexPos(float x, float y, float z = 0.f);

	VertexPos xy() const;

	LUNA_API float operator[](std::size_t i) const;
	LUNA_API float& operator[](std::size_t i);
	LUNA_API bool operator==(const VertexPos& other) const;
	LUNA_API bool operator!=(const VertexPos& other) const;
	friend std::ostream& operator<< (std::ostream& stream, const VertexPos& vertex);

private:
	float m_data[3];
};

class VertexPosColor {
public:
	LUNA_API VertexPosColor(float x, float y, float z, float r, float g, float b, float a);

	VertexPos xyz() const;
	VertexPos xy() const;

	LUNA_API float operator[](std::size_t i) const;
	LUNA_API float& operator[](std::size_t i);
	LUNA_API bool operator==(const VertexPosColor& other) const;
	LUNA_API bool operator!=(const VertexPosColor& other) const;
	friend std::ostream& operator<< (std::ostream& stream, const VertexPosColor& vertex);

private:
	float m_data[7];
};

} // luna