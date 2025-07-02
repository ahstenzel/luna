#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/vertex.hpp>

namespace luna {

namespace detail {

class Shape {
public:
	LUNA_API virtual float Area() const = 0;
};

} // detail

enum class ShapeType {
	Unknown,
	LineType,
	AABBType,
	CircleType
};

class ShapeLine : public detail::Shape {
public:
	LUNA_API ShapeLine(float _x1, float _y1, float _x2, float _y2);
	LUNA_API ShapeLine(const VertexPos& _p1, const VertexPos& _p2);

	float Area() const override;

	bool operator==(const ShapeLine& other) const;

	float x1;
	float y1;
	float x2;
	float y2;
};

class ShapeAABB : public detail::Shape {
public:
	LUNA_API ShapeAABB(float _left = 0.f, float _top = 0.f, float _right = 0.f, float _bottom = 0.f);
	LUNA_API ShapeAABB(const VertexPos& _p1, const VertexPos& _p2);

	float Area() const override;

	bool operator==(const ShapeAABB& other) const;

	float left;
	float right;
	float top;
	float bottom;
};

class ShapeCircle : public detail::Shape {
public:
	LUNA_API ShapeCircle(float _x = 0.f, float _y = 0.f, float _radius = 0.f);
	LUNA_API ShapeCircle(const VertexPos& _center, float _radius = 0.f);

	float Area() const override;

	bool operator==(const ShapeCircle& other) const;

	float x;
	float y;
	float radius;
};

LUNA_API float PointDistance(float x1, float y1, float x2, float y2);

LUNA_API float PointDistance(const VertexPos& p1, const VertexPos& p2);

LUNA_API float PointDistanceSqr(float x1, float y1, float x2, float y2);

LUNA_API float PointDistanceSqr(const VertexPos& p1, const VertexPos& p2);

LUNA_API VertexPos Midpoint(float x1, float y1, float x2, float y2);

LUNA_API VertexPos Midpoint(const VertexPos& p1, const VertexPos& p2);

LUNA_API float PointAngle(float x1, float y1, float x2, float y2);

LUNA_API float PointAngle(const VertexPos& p1, const VertexPos& p2);

LUNA_API VertexPos PerpendicularBisector(float x1, float y1, float x2, float y2, float length);

LUNA_API VertexPos PerpendicularBisector(const VertexPos& p1, const VertexPos& p2, float length);

template<typename S>
LUNA_API bool PointInShape(float x, float y, const S& shape) { return false; }

template<>
LUNA_API bool PointInShape(float x, float y, const ShapeLine& shape);

template<>
LUNA_API bool PointInShape(float x, float y, const ShapeAABB& shape);

template<>
LUNA_API bool PointInShape(float x, float y, const ShapeCircle& shape);

template<typename S1, typename S2>
LUNA_API bool ShapeIntersects(const S1& shape1, const S2& shape2) { return false; }

template<>
LUNA_API bool ShapeIntersects(const ShapeLine& shape1, const ShapeLine& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeLine& shape1, const ShapeAABB& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeLine& shape1, const ShapeCircle& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeAABB& shape1, const ShapeLine& shape2);

template<> 
LUNA_API bool ShapeIntersects(const ShapeAABB& shape1, const ShapeAABB& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeAABB& shape1, const ShapeCircle& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeCircle& shape1, const ShapeLine& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeCircle& shape1, const ShapeAABB& shape2);

template<>
LUNA_API bool ShapeIntersects(const ShapeCircle& shape1, const ShapeCircle& shape2);

using AnyShape = std::variant<std::monostate, ShapeLine, ShapeAABB, ShapeCircle>;

class Primitive {
public:
	LUNA_API Primitive();
	LUNA_API Primitive(ShapeLine shape, bool outline = false, std::int32_t depth = 0, SDL_Color blend = LunaColorWhite);
	LUNA_API Primitive(ShapeAABB shape, bool outline = false, std::int32_t depth = 0, SDL_Color blend = LunaColorWhite);
	LUNA_API Primitive(ShapeCircle shape, bool outline = false, std::int32_t depth = 0, SDL_Color blend = LunaColorWhite);
	LUNA_API Primitive(const Primitive& primitive);
	LUNA_API Primitive(Primitive&& primitive) noexcept;

	LUNA_API bool IsWireframe() const;

	LUNA_API AnyShape GetShape() const;
	LUNA_API ShapeType GetShapeType() const;
	LUNA_API bool GetOutline() const;
	LUNA_API std::int32_t GetDepth() const;
	LUNA_API SDL_Color GetBlend() const;
	LUNA_API float GetAlpha() const;

	LUNA_API std::vector<VertexPosColor> GetVertices();
	LUNA_API std::vector<std::uint16_t> GetIndices();

	LUNA_API void SetShape(ShapeLine shape);
	LUNA_API void SetShape(ShapeAABB shape);
	LUNA_API void SetShape(ShapeCircle shape);
	LUNA_API void SetOutline(bool outline);
	LUNA_API void SetDepth(std::int32_t depth);
	LUNA_API void SetBlend(SDL_Color blend);
	LUNA_API void SetAlpha(float alpha);

	LUNA_API Primitive& operator=(const Primitive& other);
	LUNA_API Primitive& operator=(Primitive&& other) noexcept;
	LUNA_API bool operator==(const Primitive& other) const;

private:
	void CalculateVertices();

	AnyShape m_shape;
	ShapeType m_shapeType;
	bool m_outline;
	std::int32_t m_depth;
	SDL_Color m_blend;
	std::vector<VertexPosColor> m_vertices;
	std::vector<std::uint16_t> m_indices;
	bool m_dirty;
};

using PrimitiveList = std::vector<Primitive>;

} // luna