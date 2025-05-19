#pragma once

#include <luna/detail/common.hpp>
#include <luna/detail/resources.hpp>

namespace luna {

typedef std::uint32_t SpriteID;
constexpr SpriteID SPRITE_ID_NULL = 0;

// Forward declarations
class SpriteList;
struct SpriteComp;

/// <summary>
/// Instance of a texture in the world, with position, scale, rotation, and animation.
/// </summary>
class Sprite {
public:
	Sprite(const ResourceID textureID, float x, float y, int32_t image = 0, int32_t depth = 0, float scaleX = 1.f, float scaleY = 1.f, float rotation = 0.f);

	bool IsValid() const;

	float GetPositionX() const;
	float GetPositionY() const;
	float GetScaleX() const;
	float GetScaleY() const;
	float GetRotation() const;
	int32_t GetImage() const;
	int32_t GetNumImages() const;
	int32_t GetDepth() const;
	ResourceID GetTextureID() const;

	void SetPositionX(float x);
	void SetPositionY(float y);
	void SetScaleX(float scaleX);
	void SetScaleY(float scaleY);
	void SetRotation(float rotation);
	void SetImage(int32_t image);
	void SetDepth(int32_t depth);

protected:
	friend class SpriteList;

	bool Tick(float dt);
	void SetSpriteList(SpriteList* spriteList);

private:
	void CalculateUVs();

	ResourceID m_textureID = RESOURCE_ID_NULL;
	float m_positionX = 0.f;
	float m_positionY = 0.f;
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
	SpriteList* m_spriteList = nullptr;
};

/// <summary>
/// Collection of sprites, sorted by different metrics.
/// </summary>
class SpriteList {
public:
	SpriteList();

	/// <summary>
	/// Add an existing sprite to this list and return its reference ID.
	/// </summary>
	/// <param name="sprite">Sprite object</param>
	/// <returns>Sprite ID</returns>
	SpriteID AddSprite(const Sprite& sprite);

	/// <summary>
	/// Create a new sprite in this list and return its reference ID.
	/// </summary>
	/// <param name="...args">Sprite constructor arguments</param>
	/// <returns>Sprite ID</returns>
	template<typename... Args>
	SpriteID CreateSprite(Args&&... args) {
		SpriteID id = SpriteList::GenerateID();
		auto result = m_sprites.emplace(std::make_pair(id, Sprite(std::move(args)...)));
		if (!result.second) { return SPRITE_ID_NULL; }
		else { result.first->second.SetSpriteList(this); }
		m_spriteOrder.push_back(id);
		m_dirty = true;
		return id;
	}

	/// <summary>
	/// Get the sprite object associated with the ID.
	/// </summary>
	/// <param name="spriteID">Sprite ID</param>
	/// <returns>Sprite object</returns>
	Sprite* GetSprite(SpriteID spriteID);

	/// <summary>
	/// Remove the sprite associated with the ID from the list.
	/// </summary>
	/// <param name="spriteID">Sprite ID</param>
	void RemoveSprite(SpriteID spriteID);

protected:
	friend class Sprite;

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