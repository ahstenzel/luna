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
	Texture2D texture;		/* Base texture */
	Vector2 position;		/* Position in worldspace */
	Vector2 scale;			/* Scaling factor */
	Vector2 origin;			/* Transformation origin (scaling, rotating etc.) */
	Color tint;				/* Blending color */
	SpriteID id;			/* Unique ID value */
	int depth;				/* Draw depth */
	int imageIndex;			/* Current animation frame */
	unsigned int imageNum;	/* Total animation frames */
	unsigned int numRows;	/* Rows of animation frames in base texture */
	unsigned int numCols;	/* Columns of animation frames in base texture */
	float imageSpeed;		/* Animation speed (seconds per frame of animation) */
	float rotation;			/* Rotation angle (degrees) */
	float _timer;			/* Internal timer for pacing animation */
	bool visible;			/* Visibility flag */
} Sprite;

/// @brief Descriptor for creating a sprite object
typedef struct {
	Texture2D texture;		/* Base texture */
	Vector2 position;		/* Position in worldspace */
	Vector2 scale;			/* Scaling factor */
	Vector2 origin;			/* Transformation origin (scaling, rotating etc.) */
	Color tint;				/* Blending color */
	int depth;				/* Draw depth */
	int imageIndex;			/* Current animation frame */
	unsigned int imageNum;	/* Total animation frames */
	unsigned int numRows;	/* Rows of animation frames in base texture */
	unsigned int numCols;	/* Columns of animation frames in base texture */
	float imageSpeed;		/* Animation speed (seconds per frame of animation) */
	float rotation;			/* Rotation angle (degrees) */
	bool visible;			/* Visibility flag */
} SpriteDesc;

/// @brief Organized list of sprites
typedef struct {
	unordered_map_t* spriteIndices;			/* Map unique sprite IDs to container indices */
	free_list_t* sprites;					/* Container of sprite data */
	priority_queue_t* spriteDepthOrder;		/* Sort container indices by depth value */
	bool depthSorting;						/* Enable depth sorting */
} SpriteList;

/// @brief Update the sprites animation.
/// @param _sprite Sprite pointer
/// @param _dt Frame time
void _UpdateSprite(Sprite* _sprite, float _dt);

/// @brief Draw the given sprite in the scene.
/// @param _sprite Sprite pointer
void _DrawSprite(Sprite* _sprite);

/// @brief Create a new sprite list.
/// @param _depthSorting Enable sorting sprites by depth
/// @return Sprite list pointer
SpriteList* _CreateSpriteList(bool _depthSorting);

/// @brief Deallocate a sprite list.
/// @param _list Sprite list pointer
void _DestroySpriteList(SpriteList* _list);

/// @brief Update all sprites in the list.
/// @param _list Sprite list pointer
/// @param _dt Frame time
void _UpdateSpriteList(SpriteList* _list, float _dt);

/// @brief Draw all sprites in the list.
/// @param _list Sprite list pointer
void _DrawSpriteList(SpriteList* _list);

/// @brief Create a new sprite and add it to the list.
/// @param _list Sprite list pointer
/// @param _desc Sprite descriptor
/// @return Sprite id
SpriteID CreateSprite(SpriteList* _list, SpriteDesc _desc);

/// @brief Remove the sprite from the list.
/// @param _list Sprite list pointer
/// @param _id Sprite id
void DestroySprite(SpriteList* _list, SpriteID _id);

/// @brief Set the sprites position.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _position Position
void SetSpritePosition(SpriteList* _list, SpriteID _id, Vector2 _position);

/// @brief Get the sprites position.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return Position
Vector2 GetSpritePosition(SpriteList* _list, SpriteID _id);

/// @brief Set the sprites draw depth.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _depth Depth
void SetSpriteDepth(SpriteList* _list, SpriteID _id, int _depth);

/// @brief Get the sprites draw depth.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return Depth
int GetSpriteDepth(SpriteList* _list, SpriteID _id);

/// @brief Set the sprites animation frame.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _imageIndex Frame index
void SetSpriteImageIndex(SpriteList* _list, SpriteID _id, int _imageIndex);

/// @brief Get the sprites animation frame.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return Frame index
int GetSpriteImageIndex(SpriteList* _list, SpriteID _id);

/// @brief Set the sprites animation speed.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _imageSpeed Length of time per frame
void SetSpriteImageSpeed(SpriteList* _list, SpriteID _id, float _imageSpeed);

/// @brief Get the sprites animation speed.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return Length of time per frame
float GetSpriteImageSpeed(SpriteList* _list, SpriteID _id);

/// @brief Set whether the sprite should be drawn.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _visible If true, sprite will be drawn
void SetSpriteVisible(SpriteList* _list, SpriteID _id, bool _visible);

/// @brief Get whether the sprite should be drawn.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return If true, sprite will be drawn
bool GetSpriteVisible(SpriteList* _list, SpriteID _id);

#endif