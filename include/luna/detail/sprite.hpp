#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/resources.hpp>

namespace luna {

typedef std::uint32_t SpriteID;
constexpr SpriteID SPRITE_ID_NULL = 0;

// Forward declarations
class RoomManager;

struct SpriteTextureCoords {
	float textureU;
	float textureV;
	float textureW;
	float textureH;
};

/// <summary>
/// Instance of a texture in the world, with position, scale, rotation, and animation.
/// </summary>
class Sprite {
public:
	LUNA_API Sprite();
	LUNA_API Sprite(const Sprite& sprite);
	LUNA_API Sprite(Sprite&& sprite) noexcept;
	LUNA_API Sprite(const ResourceID textureID, float x, float y, int32_t image = 0, float imageSpeed = 0.f, int32_t depth = 0, float scaleX = 1.f, float scaleY = 1.f, float rotation = 0.f, SDL_Color blend = LunaColorWhite);

	LUNA_API bool IsValid() const;

	LUNA_API float GetPositionX() const;
	LUNA_API float GetPositionY() const;
	LUNA_API float GetScaleX() const;
	LUNA_API float GetScaleY() const;
	LUNA_API float GetRotation() const;
	LUNA_API float GetWidth() const;
	LUNA_API float GetHeight() const;
	LUNA_API std::int32_t GetImage() const;
	LUNA_API std::int32_t GetNumImages() const;
	LUNA_API std::int32_t GetDepth() const;
	LUNA_API ResourceID GetTextureID() const;
	LUNA_API SpriteTextureCoords GetTextureCoords() const;
	LUNA_API TexturePageID GetTexturePageID() const;
	LUNA_API const TexturePage* GetTexturePage() const;
	LUNA_API SDL_Color GetBlend() const;
	LUNA_API float GetAlpha() const;
	LUNA_API float GetOriginX() const;
	LUNA_API float GetOriginY() const;
	LUNA_API bool GetTranslucent() const;

	LUNA_API void SetPositionX(float x);
	LUNA_API void SetPositionY(float y);
	LUNA_API void SetScaleX(float scaleX);
	LUNA_API void SetScaleY(float scaleY);
	LUNA_API void SetRotation(float rotation);
	LUNA_API void SetImage(std::int32_t image);
	LUNA_API void SetDepth(std::int32_t depth);
	LUNA_API void SetBlend(const SDL_Color& blend);
	LUNA_API void SetAlpha(float alpha);
	LUNA_API void SetOriginX(std::int32_t originX);
	LUNA_API void SetOriginY(std::int32_t originY);

	LUNA_API Sprite& operator=(const Sprite& other);
	LUNA_API Sprite& operator=(Sprite&& other) noexcept;
	LUNA_API bool operator==(const Sprite& other) const;

protected:
	bool Tick(float dt);

private:
	void CalculateUVs();

	const ResourceTexture* m_texture = nullptr;
	ResourceID m_textureID = RESOURCE_ID_NULL;
	SDL_Color m_blend = LunaColorWhite;
	float m_positionX = 0.f;
	float m_positionY = 0.f;
	float m_width = 0.f;
	float m_height = 0.f;
	float m_scaleX = 0.f;
	float m_scaleY = 0.f;
	float m_rotation = 0.f;
	float m_animationSpeed = 0.f;
	float m_animationFrame = 0.f;
	float m_textureU = 0.f;
	float m_textureV = 0.f;
	float m_textureW = 0.f;
	float m_textureH = 0.f;
	float m_originX = 0;
	float m_originY = 0;
	std::int32_t m_depth = 0;
};

using SpriteList = std::vector<const Sprite*>;

struct SpriteListTexturePageComp {
	bool operator()(const Sprite* lhs, const Sprite* rhs);
};

struct SpriteListDepthComp {
	bool operator()(const Sprite* lhs, const Sprite* rhs);
};


} // luna