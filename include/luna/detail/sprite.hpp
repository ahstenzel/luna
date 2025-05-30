#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/resources.hpp>

namespace luna {

typedef std::uint32_t SpriteID;
constexpr SpriteID SPRITE_ID_NULL = 0;

// Forward declarations
class RoomManager;
class SpriteList;
struct SpriteComp;

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
	LUNA_API Sprite(const ResourceID textureID, float x, float y, int32_t image = 0, int32_t depth = 0, float scaleX = 1.f, float scaleY = 1.f, float rotation = 0.f, SDL_Color blend = {255, 255, 255, 255});

	LUNA_API bool IsValid() const;

	LUNA_API float GetPositionX() const;
	LUNA_API float GetPositionY() const;
	LUNA_API float GetScaleX() const;
	LUNA_API float GetScaleY() const;
	LUNA_API float GetRotation() const;
	LUNA_API float GetWidth() const;
	LUNA_API float GetHeight() const;
	LUNA_API int32_t GetImage() const;
	LUNA_API int32_t GetNumImages() const;
	LUNA_API int32_t GetDepth() const;
	LUNA_API ResourceID GetTextureID() const;
	LUNA_API SpriteTextureCoords GetTextureCoords() const;
	LUNA_API TexturePageID GetTexturePageID() const;
	LUNA_API const TexturePage* GetTexturePage() const;
	LUNA_API SDL_Color GetBlend() const;

	LUNA_API void SetPositionX(float x);
	LUNA_API void SetPositionY(float y);
	LUNA_API void SetScaleX(float scaleX);
	LUNA_API void SetScaleY(float scaleY);
	LUNA_API void SetRotation(float rotation);
	LUNA_API void SetImage(int32_t image);
	LUNA_API void SetDepth(int32_t depth);
	LUNA_API void SetBlend(const SDL_Color& blend);

protected:
	friend class SpriteList;

	bool Tick(float dt);
	void SetSpriteList(SpriteList* spriteList);

private:
	void CalculateUVs();

	const ResourceTexture* m_texture = nullptr;
	SpriteList* m_spriteList = nullptr;
	ResourceID m_textureID = RESOURCE_ID_NULL;
	SDL_Color m_blend = {};
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
	std::int32_t m_depth = 0;
};

/// <summary>
/// Collection of sprites, sorted by different metrics.
/// </summary>
class SpriteList {
public:
	LUNA_API SpriteList();

	/// <summary>
	/// Add an existing sprite to this list and return its reference ID.
	/// </summary>
	/// <param name="sprite">Sprite object</param>
	/// <returns>Sprite ID</returns>
	LUNA_API SpriteID AddSprite(const Sprite& sprite);

	/// <summary>
	/// Create a new sprite in this list and return its reference ID.
	/// </summary>
	/// <param name="...args">Sprite constructor arguments</param>
	/// <returns>Sprite ID</returns>
	template<typename... Args>
	LUNA_API SpriteID CreateSprite(Args&&... args) {
		SpriteID id = SpriteList::GenerateID();
		auto result = m_sprites.emplace(std::make_pair(id, Sprite(std::move(args)...)));
		if (!result.second) { return SPRITE_ID_NULL; }
		else { result.first->second.SetSpriteList(this); }
		m_spriteOrder.push_back(id);
		MarkDirty();
		return id;
	}

	/// <summary>
	/// Get the sprite object associated with the ID.
	/// </summary>
	/// <param name="spriteID">Sprite ID</param>
	/// <returns>Sprite object</returns>
	LUNA_API Sprite* GetSprite(SpriteID spriteID);

	/// <summary>
	/// Get the sprite ID at the given index.
	/// </summary>
	/// <param name="idx">idx</param>
	/// <returns>Sprite ID</returns>
	LUNA_API SpriteID GetSpriteID(std::size_t idx);

	/// <summary>
	/// Remove the sprite associated with the ID from the list.
	/// </summary>
	/// <param name="spriteID">Sprite ID</param>
	LUNA_API void RemoveSprite(SpriteID spriteID);

	/// <summary>
	/// Get the number of sprites in this list.
	/// </summary>
	/// <returns>Number of sprites</returns>
	LUNA_API std::uint32_t Size() const;

	/// <summary>
	/// Sort the sprites in the list by texture page & depth, if not already sorted.
	/// </summary>
	LUNA_API void Sort();

	LUNA_API std::vector<SpriteID>::const_iterator begin() const;

	LUNA_API std::vector<SpriteID>::const_iterator end() const;

protected:
	friend class Sprite;
	friend class RoomManager;

	void MarkDirty();
	void Tick(float dt);

private:
	static SpriteID m_spriteIDCounter;
	static SpriteID GenerateID();

	struct SpriteComp {
		std::unordered_map<SpriteID, Sprite>* m_list;
		bool operator()(SpriteID lhs, SpriteID rhs);
	};
	SpriteComp m_comparator;
	std::unordered_map<SpriteID, Sprite> m_sprites;
	std::vector<SpriteID> m_spriteOrder;
	bool m_dirty = false;
};

} // luna