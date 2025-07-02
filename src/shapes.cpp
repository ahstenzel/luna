#include <luna/detail/shapes.hpp>

namespace luna {

ShapeLine::ShapeLine(float _x1, float _y1, float _x2, float _y2) :
	x1(_x1),
	y1(_y1), 
	x2(_x2), 
	y2(_y2) {}

ShapeLine::ShapeLine(const VertexPos& _p1, const VertexPos& _p2) :
	x1(_p1[0]),
	y1(_p1[1]), 
	x2(_p2[0]), 
	y2(_p2[1]) {}

float ShapeLine::Area() const {
	return PointDistance(x1, y1, x2, y2);
}

bool ShapeLine::operator==(const ShapeLine& other) const {
	return (
		x1 == other.x1 &&
		x2 == other.x2 &&
		y1 == other.y1 &&
		y2 == other.y2
		);
}

ShapeAABB::ShapeAABB(float _left, float _top, float _right, float _bottom) :
	left(_left),
	top(_top),
	right(_right),
	bottom(_bottom) {}

ShapeAABB::ShapeAABB(const VertexPos& _p1, const VertexPos& _p2) :
	left(_p1[0]),
	top(_p1[1]),
	right(_p2[0]),
	bottom(_p2[1]) { }

float ShapeAABB::Area() const {
	return (right - left) * (bottom - top);
}

bool ShapeAABB::operator==(const ShapeAABB& other) const {
	return (
		left == other.left &&
		top == other.top &&
		right == other.right &&
		bottom == other.bottom
		);
}

ShapeCircle::ShapeCircle(float _x, float _y, float _radius) :
	x(_x),
	y(_y),
	radius(_radius) {}

ShapeCircle::ShapeCircle(const VertexPos& _center, float _radius) :
	x(_center[0]),
	y(_center[1]),
	radius(_radius) {}

float ShapeCircle::Area() const {
	return radius * radius * PI_F;
}

bool ShapeCircle::operator==(const ShapeCircle& other) const {
	return (
		x == other.x && 
		y == other.y &&
		radius == other.radius
		);
}

float PointDistance(float x1, float y1, float x2, float y2) {
	float xx = (x2 - x1);
	float yy = (y2 - y1);
	return std::sqrtf(xx * xx + yy * yy);
}

float PointDistance(const VertexPos& p1, const VertexPos& p2) {
	return PointDistance(p1[0], p1[1], p2[0], p2[1]);
}

float PointDistanceSqr(float x1, float y1, float x2, float y2) {
	float xx = (x2 - x1);
	float yy = (y2 - y1);
	return (xx * xx + yy * yy);
}

float PointDistanceSqr(const VertexPos& p1, const VertexPos& p2) {
	return PointDistanceSqr(p1[0], p1[1], p2[0], p2[1]);
}

VertexPos Midpoint(float x1, float y1, float x2, float y2) {
	return VertexPos(
		((x2 - x1) / 2.f) + x1,
		((y2 - y1) / 2.f) + y1
	);
}

VertexPos Midpoint(const VertexPos& p1, const VertexPos& p2) {
	return Midpoint(p1[0], p1[1], p2[0], p2[1]);
}

float PointAngle(float x1, float y1, float x2, float y2) {
	return SDL_atan2f(y2 - y1, x2 - x1);
}

float PointAngle(const VertexPos& p1, const VertexPos& p2) {
	return PointAngle(p1[0], p1[1], p2[0], p2[1]);
}

VertexPos PerpendicularBisector(float x1, float y1, float x2, float y2, float length) {
	float dir = PointAngle(x1, y1, x2, y2) + (PI_2_F);
	VertexPos mid = Midpoint(x1, y1, x2, y2);
	return VertexPos(
		mid[0] + (length * SDL_sinf(dir)),
		mid[1] + (length * SDL_cosf(dir))
	);
}

VertexPos PerpendicularBisector(const VertexPos& p1, const VertexPos& p2, float length) {
	return PerpendicularBisector(p1[0], p1[1], p2[0], p2[1], length);
}

template<>
bool PointInShape(float x, float y, const ShapeLine& shape) {
	float d1 = PointDistance(x, y, shape.x1, shape.y1);
	float d2 = PointDistance(x, y, shape.x2, shape.y2);
	return (d1 + d2) == shape.Area();
}

template<>
bool PointInShape(float x, float y, const ShapeAABB& shape) {
	return (
		x >= shape.left && x <= shape.right &&
		y >= shape.top && y <= shape.bottom
		);
}

template<>
bool PointInShape(float x, float y, const ShapeCircle& shape) {
	float dx = shape.x - x;
	float dy = shape.y - y;
	return (shape.radius * shape.radius) <= (dx * dx + dy * dy);
}

template<>
bool ShapeIntersects(const ShapeLine& shape1, const ShapeLine& shape2) {
	float d  = (shape2.y2 - shape2.y1) * (shape1.x2 - shape1.x1) - (shape2.x2 - shape2.x1) * (shape1.y2 - shape1.y1);
	float n1 = (shape2.x2 - shape2.x1) * (shape1.y1 - shape2.y1) - (shape2.y2 - shape2.y1) * (shape1.x1 - shape2.x1);
	float n2 = (shape1.x2 - shape1.x1) * (shape1.y1 - shape2.y1) - (shape1.y2 - shape1.y1) * (shape1.x1 - shape2.x1);
	if (n1 == 0.f && n2 == 0.f) {
		if (d == 0.f) { return true; }	// Lines are coincident
		else { return false; }	// Lines are parallel
	}
	float u1 = n1 / d;
	float u2 = n2 / d;
	return (u1 >= 0.f && u1 <= 1.f && u2 >= 0.f && u2 <= 1.f);
}

template<>
bool ShapeIntersects(const ShapeLine& shape1, const ShapeAABB& shape2) {
	bool left = ShapeIntersects(ShapeLine(shape2.left, shape2.top, shape2.left, shape2.bottom), shape1);
	bool top = ShapeIntersects(ShapeLine(shape2.left, shape2.top, shape2.right, shape2.top), shape1);
	bool right = ShapeIntersects(ShapeLine(shape2.right, shape2.top, shape2.right, shape2.bottom), shape1);
	bool bottom = ShapeIntersects(ShapeLine(shape2.left, shape2.bottom, shape2.right, shape2.bottom), shape1);
	return (left || top || right || bottom);
}

template<>
bool ShapeIntersects(const ShapeLine& shape1, const ShapeCircle& shape2) {
	bool p1 = PointInShape(shape1.x1, shape1.y1, shape2);
	bool p2 = PointInShape(shape1.x2, shape1.y2, shape2);
	if (p1 || p2) { return true; }
	float len = shape1.Area();
	float dot = (((shape2.x - shape1.x1) * (shape1.x2 - shape1.x1)) + ((shape2.y - shape1.y1) * (shape1.y2 - shape1.y1))) / (len * len);
	float nx = shape1.x1 + (dot * (shape1.x2 - shape1.x1));
	float ny = shape1.y1 + (dot * (shape1.y2 - shape1.y1));
	if (!PointInShape(nx, ny, shape1)) { return false; }
	float dist = PointDistanceSqr(nx, ny, shape2.x, shape2.y);
	return (dist <= (shape2.radius * shape2.radius));
}

template<>
bool ShapeIntersects(const ShapeAABB& shape1, const ShapeLine& shape2) {
	return ShapeIntersects(shape2, shape1);
}

template<>
bool ShapeIntersects(const ShapeAABB& shape1, const ShapeAABB& shape2) {
	return (
		shape1.right >= shape2.left &&
		shape1.left <= shape2.right &&
		shape1.bottom >= shape2.top &&
		shape1.top <= shape2.bottom
		);
}

template<>
bool ShapeIntersects(const ShapeAABB& shape1, const ShapeCircle& shape2) {
	float cx = (shape2.x < shape1.left ? shape1.left : (shape2.x > shape1.right ? shape1.right : shape2.x));
	float cy = (shape2.y < shape1.top ? shape1.top : (shape2.y > shape1.bottom ? shape1.bottom : shape2.y));
	float dx = cx - shape2.x;
	float dy = cy - shape2.y;
	return (dx * dx + dy * dy) <= shape2.radius * shape2.radius;
}

template<>
bool ShapeIntersects(const ShapeCircle& shape1, const ShapeLine& shape2) {
	return ShapeIntersects(shape2, shape1);
}

template<>
bool ShapeIntersects(const ShapeCircle& shape1, const ShapeAABB& shape2) {
	return ShapeIntersects(shape2, shape1);
}

template<>
bool ShapeIntersects(const ShapeCircle& shape1, const ShapeCircle& shape2) {
	float dx = shape2.x - shape1.x;
	float dy = shape2.y - shape1.y;
	float d = shape1.radius + shape2.radius;
	return (d * d) <= (dx * dx + dy * dy);
}

Primitive::Primitive() :
	m_shape(),
	m_shapeType(ShapeType::Unknown),
	m_outline(false),
	m_depth(0),
	m_blend(LunaColorWhite),
	m_dirty(true) {}

Primitive::Primitive(ShapeLine shape, bool outline, std::int32_t depth, SDL_Color blend) :
	m_shape(shape),
	m_shapeType(ShapeType::LineType),
	m_outline(outline),
	m_depth(depth),
	m_blend(blend),
	m_dirty(true) {}

Primitive::Primitive(ShapeAABB shape, bool outline, std::int32_t depth, SDL_Color blend) :
	m_shape(shape),
	m_shapeType(ShapeType::AABBType),
	m_outline(outline),
	m_depth(depth),
	m_blend(blend),
	m_dirty(true) {}

Primitive::Primitive(ShapeCircle shape, bool outline, std::int32_t depth, SDL_Color blend) :
	m_shape(shape),
	m_shapeType(ShapeType::CircleType),
	m_outline(outline),
	m_depth(depth),
	m_blend(blend),
	m_dirty(true) {}

Primitive::Primitive(const Primitive& primitive) :
	m_shape(primitive.m_shape),
	m_shapeType(primitive.m_shapeType),
	m_outline(primitive.m_outline),
	m_depth(primitive.m_depth),
	m_blend(primitive.m_blend),
	m_dirty(primitive.m_dirty) {}

Primitive::Primitive(Primitive&& primitive) noexcept :
	m_shape(std::move(primitive.m_shape)),
	m_shapeType(std::move(primitive.m_shapeType)),
	m_outline(std::move(primitive.m_outline)),
	m_depth(std::move(primitive.m_depth)),
	m_blend(std::move(primitive.m_blend)),
	m_dirty(std::move(primitive.m_dirty)) {}

bool Primitive::IsWireframe() const {
	return m_shapeType == ShapeType::LineType || m_outline;
}

AnyShape Primitive::GetShape() const {
	return m_shape;
}

ShapeType Primitive::GetShapeType() const {
	return m_shapeType;
}

bool Primitive::GetOutline() const {
	return m_outline;
}

std::int32_t Primitive::GetDepth() const {
	return m_depth;
}

SDL_Color Primitive::GetBlend() const {
	return m_blend;
}

float Primitive::GetAlpha() const {
	return m_blend.a / 255.f;
}

std::vector<VertexPosColor> Primitive::GetVertices() {
	CalculateVertices();
	return m_vertices;
}

std::vector<std::uint16_t> Primitive::GetIndices() {
	CalculateVertices();
	return m_indices;
}

void Primitive::SetShape(ShapeLine shape) {
	m_shape = shape;
	m_shapeType = ShapeType::LineType;
	m_dirty = true;
}

void Primitive::SetShape(ShapeAABB shape) {
	m_shape = shape;
	m_shapeType = ShapeType::AABBType;
	m_dirty = true;
}

void Primitive::SetShape(ShapeCircle shape) {
	m_shape = shape;
	m_shapeType = ShapeType::CircleType;
	m_dirty = true;
}

void Primitive::SetOutline(bool outline) {
	m_outline = outline;
	m_dirty = true;
}

void Primitive::SetDepth(std::int32_t depth) {
	m_depth = depth;
}

void Primitive::SetBlend(SDL_Color blend) {
	m_blend = blend;
}

void Primitive::SetAlpha(float alpha) {
	m_blend.a = Uint8(alpha * 255.f);
}

Primitive& Primitive::operator=(const Primitive& other) {
	if (this == &other) { return *this; }
	m_shape = other.m_shape;
	m_shapeType = other.m_shapeType;
	m_outline = other.m_outline;
	m_depth = other.m_depth;
	m_blend = other.m_blend;
	m_vertices = other.m_vertices;
	m_indices = other.m_indices;
	return *this;
}

Primitive& Primitive::operator=(Primitive&& other) noexcept {
	if (this == &other) { return *this; }
	m_shape = std::move(other.m_shape);
	m_shapeType = std::move(other.m_shapeType);
	m_outline = std::move(other.m_outline);
	m_depth = std::move(other.m_depth);
	m_blend = std::move(other.m_blend);
	m_vertices = std::move(other.m_vertices);
	m_indices = std::move(other.m_indices);
	return *this;
}

bool Primitive::operator==(const Primitive& other) const {
	return (
		m_shape == other.m_shape &&
		m_shapeType == other.m_shapeType &&
		m_outline == other.m_outline &&
		m_depth == other.m_depth &&
		m_blend.r == other.m_blend.r &&
		m_blend.g == other.m_blend.g &&
		m_blend.b == other.m_blend.b &&
		m_blend.a == other.m_blend.a &&
		m_vertices == other.m_vertices &&
		m_indices == other.m_indices
		);
}

void Primitive::CalculateVertices() {
	if (!m_dirty) { return; }
	m_dirty = false;
	m_vertices.clear();
	m_indices.clear();
	SDL_FColor color = ConvertToFColor(m_blend);
	float z = -float(m_depth);
	switch (m_shapeType) {
	case ShapeType::LineType: {
		ShapeLine shapeLine = std::get<ShapeLine>(m_shape);
		m_vertices.push_back(VertexPosColor(shapeLine.x1, shapeLine.y1, z, color.r, color.g, color.b, color.a));
		m_vertices.push_back(VertexPosColor(shapeLine.x2, shapeLine.y2, z, color.r, color.g, color.b, color.a));
		m_indices.push_back(0);
		m_indices.push_back(1);
	} break;
	case ShapeType::AABBType: {
		ShapeAABB shapeAABB = std::get<ShapeAABB>(m_shape);
		if (m_outline) {
			m_vertices.push_back(VertexPosColor(shapeAABB.left, shapeAABB.top, z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(shapeAABB.left, shapeAABB.bottom, z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(shapeAABB.right, shapeAABB.bottom, z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(shapeAABB.right, shapeAABB.top, z, color.r, color.g, color.b, color.a));
			m_indices.push_back(0);
			m_indices.push_back(1);
			m_indices.push_back(1);
			m_indices.push_back(2);
			m_indices.push_back(2);
			m_indices.push_back(3);
			m_indices.push_back(3);
			m_indices.push_back(0);
		}
		else {
			m_vertices.push_back(VertexPosColor(shapeAABB.left, shapeAABB.top, z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(shapeAABB.left, shapeAABB.bottom, z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(shapeAABB.right, shapeAABB.top, z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(shapeAABB.right, shapeAABB.bottom, z, color.r, color.g, color.b, color.a));
			m_indices.push_back(0);
			m_indices.push_back(1);
			m_indices.push_back(2);
			m_indices.push_back(3);
			m_indices.push_back(2);
			m_indices.push_back(1);
		}
	} break;
	case ShapeType::CircleType: {
		ShapeCircle shapeCircle = std::get<ShapeCircle>(m_shape);
		VertexPos center(shapeCircle.x, shapeCircle.y);
		if (m_outline) {
			float min_angle = 1.0f / shapeCircle.radius;

			// Push initial point
			m_indices.push_back(0);
			m_vertices.push_back(VertexPosColor(center[0] + shapeCircle.radius, center[1], z, color.r, color.g, color.b, color.a));

			// Rotate around, pushing more points
			float accum = 0.f;
			while ((TAU_F - accum) > min_angle) {
				accum += min_angle;
				float dx = shapeCircle.radius * SDL_cosf(accum);
				float dy = shapeCircle.radius * SDL_sinf(accum);
				m_indices.push_back(m_vertices.size());
				m_indices.push_back(m_vertices.size());
				m_vertices.push_back(VertexPosColor(center[0] + dx, center[1] + dy, z, color.r, color.g, color.b, color.a));
			}

			// Connect back to first point
			m_indices.push_back(0);
		}
		else {
			std::queue<std::pair<std::size_t, std::size_t>> edges;

			// Create equilateral triangle
			float a = shapeCircle.radius / 2.f;
			float b = (shapeCircle.radius * SDL_sqrtf(3.f)) / 2.f;
			VertexPos p1(shapeCircle.x, shapeCircle.y - shapeCircle.radius);
			VertexPos p2(shapeCircle.x - b, shapeCircle.y + a);
			VertexPos p3(shapeCircle.x + b, shapeCircle.y + a);
			m_vertices.push_back(VertexPosColor(p1[0], p1[1], z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(p2[0], p2[1], z, color.r, color.g, color.b, color.a));
			m_vertices.push_back(VertexPosColor(p3[0], p3[1], z, color.r, color.g, color.b, color.a));
			m_indices.push_back(0);
			m_indices.push_back(1);
			m_indices.push_back(2);
			edges.push(std::make_pair(0, 1));
			edges.push(std::make_pair(1, 2));
			edges.push(std::make_pair(2, 0));

			// Iterate on triangle edges
			while (!edges.empty()) {
				// Get midpoint of triangle side
				auto& pair = edges.front();
				VertexPos p1 = m_vertices[pair.first].xy();
				VertexPos p2 = m_vertices[pair.second].xy();
				VertexPos pm = Midpoint(p1, p2);

				// Check for end condition
				float dist = PointDistance(center, pm);
				if ((shapeCircle.radius - dist) < 1.f) { break; }

				// Project midpoint to circumference
				float dir = PointAngle(center, pm);
				float dx = shapeCircle.radius * SDL_cosf(dir);
				float dy = shapeCircle.radius * SDL_sinf(dir);
				VertexPos p3(center[0] + dx, center[1] + dy);

				// Add new point to vertex list
				std::size_t new_index = m_vertices.size();
				m_vertices.push_back(VertexPosColor(p3[0], p3[1], z, color.r, color.g, color.b, color.a));
				m_indices.push_back(pair.first);
				m_indices.push_back(pair.second);
				m_indices.push_back(new_index);

				// Add two new edges to the queue
				edges.push(std::make_pair(new_index, pair.first));
				edges.push(std::make_pair(pair.second, new_index));
				edges.pop();
			}
		}
	} break;
	}
}

} // luna