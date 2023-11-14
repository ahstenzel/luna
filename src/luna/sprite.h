#ifndef LUNA_SPRITE_H
#define LUNA_SPRITE_H
/**
 * sprite.h
 * Animated texture object.
*/
#include "luna/common.h"

/// @brief Unique sprite identifier
typedef _LUNA_ID_TYPE SpriteID;

/// @brief Single drawable object
typedef struct {
	Texture2D texture;
	Vector2 position;
	Color tint;
	int depth;
	int imageIndex;
	unsigned int id;
	unsigned int imageNum;
	unsigned int numRows;
	unsigned int numCols;
	float imageSpeed;
	float timer;
	bool visible;
} Sprite;

/// @brief Descriptor for creating a sprite object
typedef struct {
	Texture2D texture;
	Vector2 position;
	Color tint;
	int depth;
	int imageIndex;
	unsigned int imageNum;
	unsigned int numRows;
	unsigned int numCols;
	float imageSpeed;
	bool visible;
} SpriteDesc;

/// @brief Organized list of sprites
typedef struct {
	unordered_map_t* spriteIndices;
	free_list_t* sprites;
	priority_queue_t* spriteDepthOrder;
	bool depthSorting;
} SpriteList;

/// @brief Update the sprites animation.
/// @param _sprite Sprite pointer
/// @param _dt Frame time
void UpdateSprite(Sprite* _sprite, float _dt);

/// @brief Draw the given sprite in the scene.
/// @param _sprite Sprite pointer
void DrawSprite(Sprite* _sprite);

/// @brief Create a new sprite list.
/// @param _depthSorting Enable sorting sprites by depth (position z-component).
/// @return Sprite list pointer
SpriteList* CreateSpriteList(bool _depthSorting);

/// @brief Deallocate a sprite list.
/// @param _list Sprite list pointer
void DestroySpriteList(SpriteList* _list);

/// @brief Update all sprites in the list.
/// @param _list Sprite list pointer
/// @param _dt Frame time
void UpdateSpriteList(SpriteList* _list, float _dt);

/// @brief Draw all sprites in the list.
/// @param _list Sprite list pointer
void DrawSpriteList(SpriteList* _list);

/// @brief Create a new sprite and add it to the list.
/// @param _list Sprite list pointer
/// @param _desc Sprite descriptor
/// @return Sprite id
SpriteID CreateSprite(SpriteList* _list, SpriteDesc _desc);

/// @brief Remove the sprite from the list.
/// @param _list Sprite list pointer
/// @param _id Sprite id
void DestroySprite(SpriteList* _list, SpriteID _id);

/// @brief Change the sprites position.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _position Position
void SetSpritePosition(SpriteList* _list, SpriteID _id, Vector2 _position);

/// @brief Change the sprites draw depth.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _depth Depth
void SetSpriteDepth(SpriteList* _list, SpriteID _id, int _depth);

/// @brief Set the sprites animation frame.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _imageIndex Frame index
void SetSpriteImageIndex(SpriteList* _list, SpriteID _id, int _imageIndex);

/// @brief Set the sprites animation speed.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _imageSpeed Length of time per frame
void SetSpriteImageSpeed(SpriteList* _list, SpriteID _id, float _imageSpeed);

/// @brief Set whether the sprite should be drawn.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _visible If true, sprite will be drawn
void SetSpriteVisible(SpriteList* _list, SpriteID _id, bool _visible);

#endif