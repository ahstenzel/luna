#include <luna/detail/vertex.hpp>

namespace luna {

VertexPos::VertexPos(float x, float y, float z) {
	m_data[0] = x;
	m_data[1] = y;
	m_data[2] = z;
}

float VertexPos::operator[](std::size_t i) const {
	return m_data[i];
}

float& VertexPos::operator[](std::size_t i) {
	return m_data[i];
}

bool VertexPos::operator==(const VertexPos& other) const {
	return (
		m_data[0] == other.m_data[0] &&
		m_data[1] == other.m_data[1] &&
		m_data[2] == other.m_data[2]
		);
}

bool VertexPos::operator!=(const VertexPos& other) const {
	return !(*this == other);
}

std::ostream& operator<<(std::ostream& stream, const VertexPos& vertex) {
	stream << "{" << vertex[0] << "," << vertex[1] << "," << vertex[2] << "}";
	return stream;
}

VertexPosColor::VertexPosColor(float x, float y, float z, float r, float g, float b, float a) {
	m_data[0] = x;
	m_data[1] = y;
	m_data[2] = z;
	m_data[3] = r;
	m_data[4] = g;
	m_data[5] = b;
	m_data[6] = a;
}

float VertexPosColor::operator[](std::size_t i) const {
	return m_data[i];
}

float& VertexPosColor::operator[](std::size_t i) {
	return m_data[i];
}

bool VertexPosColor::operator==(const VertexPosColor& other) const {
	return (
		m_data[0] == other.m_data[0] &&
		m_data[1] == other.m_data[1] &&
		m_data[2] == other.m_data[2] &&
		m_data[3] == other.m_data[3] &&
		m_data[4] == other.m_data[4] &&
		m_data[5] == other.m_data[5] &&
		m_data[6] == other.m_data[6]
		);
}


bool VertexPosColor::operator!=(const VertexPosColor& other) const {
	return !(*this == other);
}

std::ostream& operator<<(std::ostream& stream, const VertexPosColor& vertex) {
	stream << std::fixed << std::setprecision(2) << "{" << vertex[0] << "," << vertex[1] << "," << vertex[2] << " / "
		<< vertex[3] << "," << vertex[4] << "," << vertex[5] << "," << vertex[6] << "}";
	return stream;
}

} // luna