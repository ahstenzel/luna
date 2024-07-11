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
	Texture2D _texture;			// Base texture
	Vector2 _position;			// Position in worldspace
	Vector2 _scale;				// Scaling factor
	Vector2 _origin;			// Transformation origin (scaling, rotating etc.)
	Vector2 _scrollSpeed;		// Sprite lateral scrolling speed (pixels / sec)
	Vector2 _scrollOffset;		// Current offset for scrolling
	Vector2 _scrollTimer;		// Internal timer for pacing lateral scrolling
	Color _tint;				// Blending color
	const SpriteID _id;			// Unique sprite ID
	int _depth;					// Draw depth
	int _imageIndex;			// Current animation frame
	unsigned int _imageNum;		// Total animation frames
	unsigned int _numRows;		// Rows of animation frames in base texture
	unsigned int _numCols;		// Columns of animation frames in base texture
	float _imageSpeed;			// Animation speed (frames / sec)
	float _rotation;			// Rotation angle (degrees)
	float _timer;				// Internal timer for pacing animation
	bool _visible;				// Whether to draw the sprite
} Sprite;

/// @brief Descriptor for creating a sprite object
typedef struct {
	Texture2D texture;			// Base texture
	Vector2 position;			// Position in worldspace
	Vector2 scale;				// Scaling factor
	Vector2 origin;				// Transformation origin (scaling, rotating etc.)
	Vector2 scrollSpeed;		// Sprite lateral scrolling speed (pixels / sec)
	Vector2 scrollOffset;		// Current offset for sprite lateral scrolling
	Color tint;					// Blending color
	int depth;					// Draw depth
	int imageIndex;				// Current animation frame
	unsigned int imageNum;		// Total animation frames
	unsigned int numRows;		// Rows of animation frames in base texture
	unsigned int numCols;		// Columns of animation frames in base texture
	float imageSpeed;			// Animation speed (frames / sec)
	float rotation;				// Rotation angle (degrees)
	bool visible;				// Whether to draw the sprite
} SpriteDesc;

/// @brief Organized list of sprites
typedef struct {
	unordered_map_t* _spriteIndices;		// Map sprite IDs to container indices
	free_list_t* _sprites;					// Container of sprite data
	priority_queue_t* _spriteDepthOrder;	// Sort container indices by depth value
	bool _depthSorting;						// Enable depth sorting
} SpriteList;

/// @brief Iterator for sprite objects
typedef struct {
	SpriteList* _list;			// Sprite list to iterate through
	unordered_map_it_t* _ptr;	// Internal iterator reference
	SpriteID id;				// Unique sprite ID
	Sprite* data;				// Sprite data structure
} SpriteListIt;

/// @brief Iterator for sprite objects sorted by depth
typedef struct {
	SpriteList* _list;			// Sprite list to iterate through
	priority_queue_it_t* _ptr;	// Internal iterator reference
	SpriteID id;				// Unique sprite ID
	Sprite* data;				// Sprite data structure
} SpriteListDepthIt;

/// @brief Generate a sprite descriptor populating a lot of fields with default values.
#define LUNA_SPRITE_DESC(_texture, _position, _depth) { \
	.texture = _texture, \
	.position = _position, \
	.scale = { 1.f, 1.f }, \
	.origin = { 0.f, 0.f }, \
	.scrollSpeed = { 0.f, 0.f }, \
	.scrollOffset = { 0.f, 0.f }, \
	.tint = WHITE, \
	.depth = _depth, \
	.imageIndex = 0, \
	.imageNum = 1, \
	.numRows = 1, \
	.numCols = 1, \
	.imageSpeed = 0.f, \
	.rotation = 0.f, \
	.visible = true \
}

/// @brief Generate a sprite descriptor populating some fields with default values.
#define LUNA_SPRITE_DESC_PRO(_texture, _position, _depth, _imageIndex, _imageSpeed, _imageNum, _numRows, _numCols) { \
	.texture = _texture, \
	.position = _position, \
	.scale = { 1.f, 1.f }, \
	.origin = { 0.f, 0.f }, \
	.scrollSpeed = { 0.f, 0.f }, \
	.scrollOffset = { 0.f, 0.f }, \
	.tint = WHITE, \
	.depth = _depth, \
	.imageIndex = _imageIndex, \
	.imageNum = _imageNum, \
	.numRows = _numRows, \
	.numCols = _numCols, \
	.imageSpeed = _imageSpeed, \
	.rotation = 0.f, \
	.visible = true \
}

/// @brief Calculate a sprites width, including its scaling.
/// @param _sprite Sprite pointer
#define LUNA_SPRITE_WIDTH(_sprite) (_sprite->_texture.width * _sprite->_scale.x / (float)_sprite->_numCols)

/// @brief Calculate a sprites height, including its scaling.
/// @param _sprite Sprite pointer
#define LUNA_SPRITE_HEIGHT(_sprite) (_sprite->_texture.height * _sprite->_scale.y / (float)_sprite->_numRows)

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

/// @brief Get the number of sprites in the list.
/// @param _list Sprite list pointer
/// @return Number of sprites
size_t GetSpriteListSize(SpriteList* _list);

/// @brief Get an iterator for the unsorted sprite list.
/// @param _list Sprite list pointer
/// @return Sprite iterator
SpriteListIt* SpriteListItBegin(SpriteList* _list);

/// @brief Move the iterator to the next unsorted element.
/// @param _it Sprite iterator
/// @return Updated sprite iterator (or NULL)
SpriteListIt* SpriteListItNext(SpriteListIt* _it);

/// @brief Get an iterator to the beginning of the sprite list (sorted by depth order).
/// @param _list Sprite list pointer
/// @return Sprite iterator
SpriteListDepthIt* SpriteListDepthItBegin(SpriteList* _list);

/// @brief Get an iterator to the end of the sprite list (sorted by depth order).
/// @param _list Sprite list pointer
/// @return Sprite iterator
SpriteListDepthIt* SpriteListDepthItRBegin(SpriteList* _list);

/// @brief Move the iterator to the next element (sorted by depth order).
/// @param _it Sprite iterator
/// @return Updated sprite iterator (or NULL)
SpriteListDepthIt* SpriteListDepthItNext(SpriteListDepthIt* _it);

/// @brief Move the iterator to the previous element (sorted by depth order).
/// @param _it Sprite iterator
/// @return Updated sprite iterator (or NULL)
SpriteListDepthIt* SpriteListDepthItPrev(SpriteListDepthIt* _it);

/// @brief Create a new sprite and add it to the list.
/// @param _list Sprite list pointer
/// @param _desc Sprite descriptor
/// @return Sprite id (or ID_NULL on error)
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

/// @brief Set the sprites scroll speed.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _scrollSpeed Lateral scroll speed
void SetSpriteScrollSpeed(SpriteList* _list, SpriteID _id, Vector2 _scrollSpeed);

/// @brief Get the sprites scroll speed.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return Scroll speed
Vector2 GetSpriteScrollSpeed(SpriteList* _list, SpriteID _id);

/// @brief Set the sprites scroll offset.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @param _scrollOffset Scroll offset
void SetSpriteScrollOffset(SpriteList* _list, SpriteID _id, Vector2 _scrollOffset);

/// @brief Get the sprites scroll offset.
/// @param _list Sprite list pointer
/// @param _id Sprite id
/// @return Scroll offset
Vector2 GetSpriteScrollOffset(SpriteList* _list, SpriteID _id);

#endif