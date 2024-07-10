#ifndef LUNA_TILE_H
#define LUNA_TILE_H
/**
 * tile.h
 * Scene tile map object.
*/
#include "luna/common.h"

/// @brief Unique tilemap identifier
typedef _LUNA_ID_TYPE TilemapID;

/// @brief Tile index type
typedef unsigned int TileIdx;

#define TILE_NULL (TileIdx)0

/// @brief Check if the given grid square is within the bounds of the scenes tile map.
/// @param t Tilemap object pointer
/// @param v Tilemap scene grid square
#define _tile_valid_grid(t, v) ((v).x >= 0 && (v).x < (t)->sceneMapSize.x && (v).y >= 0 && (v).y < (t)->sceneMapSize.y)

/// @brief Check if the given index is valid within the base texture of the tilemap.
/// @param t Tilemap object pointer
/// @param i Tile index
#define _tile_valid_idx(t, i) ((i) >= 0 && (int32_t)(i) < (((t)->texMapSize.x * (t)->texMapSize.y) + 1))

/// @brief 2D grid of tiles
typedef struct {
	Texture2D texture;			// Base texture
	Color tint;					// Color tint
	Vector2i texOffset;			// Offset (in pixels) of first tile in the texture
	Vector2i sceneOffset;		// Offset (in pixels) of the tile map in the scene
	Vector2i texTileSize;		// Size (in pixels) of a single tile in the texture
	Vector2i texTileSpacing;	// Space (in pixels) between tiles in the texture
	Vector2i sceneMapSize;		// Size (in grid squares) of the tile map in the scene
	Vector2i texMapSize;		// Size (in grid squares) of the tile map in the texture
	vector_t* _data;			// Container of tile data
	TilemapID _id;				// Unique tilemap ID
	int depth;					// Draw depth
	bool visible;				// Visibility flag
} Tilemap;

/// @brief Descriptor for creating a tilemap object
typedef struct {
	Texture2D texture;			// Base texture
	Color tint;					// Color tint
	Vector2i texOffset;			// Offset (in pixles) of first tile in the texture
	Vector2i sceneOffset;		// Offset (in pixels) of the tile map in the scene
	Vector2i texTileSize;		// Size (in pixels) of a single tile in the texture
	Vector2i texTileSpacing;	// Space (in pixels) between tiles in the texture
	Vector2i sceneMapSize;		// Size (in grid squares) of the tile map in the scene
	int depth;					// Draw depth
	bool visible;				// Visibility flag
} TilemapDesc;

/// @brief Organized list of tilemaps
typedef struct {
	unordered_map_t* tilemapIndices;		// Map tilemap IDs to container indices
	free_list_t* tilemaps;					// Container of tilemap data
	priority_queue_t* tilemapDepthOrder;	// Sort tilemap indices by depth value
	bool depthSorting;						// Enable depth sorting
} TilemapList;

/// @brief Iterator for tilemap objects
typedef struct {
	TilemapList* _list;
	unordered_map_it_t* _ptr;
	TilemapID id;		// Unique tilemap ID
	Tilemap* data;		// Tilemap data structure
} TilemapListIt;

/// @brief Iterator for tilemap objects (sorted by depth)
typedef struct {
	TilemapList* _list;
	priority_queue_it_t* _ptr;
	TilemapID id;		// Unique tilemap ID
	Tilemap* data;		// Tilemap data structure
} TilemapListDepthIt;

/// @brief Draw the tilemap.
/// @param _tile Tile object pointer
void _DrawTilemap(Tilemap* _tile);

/// @brief Create a new tilemap list.
/// @param depthSorting Enable sorting tilemaps by depth
/// @return Tilemap list pointer
TilemapList* _CreateTilemapList(bool depthSorting);

/// @brief Deallocate a tilemap list.
/// @param _list Tilemap list pointer
void _DestroyTilemapList(TilemapList* _list);

/// @brief Draw all tilemaps in the list.
/// @param _list Tilemap list pointer
void _DrawTilemapList(TilemapList* _list);

/// @brief Get the number of tilemaps in the list.
/// @param _list Tilemap list pointer
/// @return Number of tilemaps
size_t GetTilemapSize(TilemapList* _list);

/// @brief Get an iterator for the unsorted tilemap list.
/// @param _list Tilemap list pointer
/// @return Tilemap iterator
TilemapListIt* TilemapListItBegin(TilemapList* _list);

/// @brief Move the iterator to the next unsorted element.
/// @param _it Pointer to tilemap iterator
void TilemapListItNext(TilemapListIt** _it);

/// @brief Get an iterator to the beginning of the tilemap list (sorted by depth order).
/// @param _list Tilemap list pointer
/// @return Tilemap iterator
TilemapListDepthIt* TilemapListDepthItBegin(TilemapList* _list);

/// @brief Get an iterator to the end of the tilemap list (sorted by depth order).
/// @param _list Tilemap list pointer
/// @return Tilemap iterator
TilemapListDepthIt* TilemapListDepthItRBegin(TilemapList* _list);

/// @brief Move the iterator to the next element (sorted by depth order).
/// @param _it Pointer to tilemap iterator
void TilemapListDepthItNext(TilemapListDepthIt** _it);

/// @brief Move the iterator to the previous element (sorted by depth order).
/// @param _it Pointer to tilemap iterator
void TilemapListDepthItPrev(TilemapListDepthIt** _it);

/// @brief Create a new tilemap object and add it to the list.
/// @param _list Tilemap list pointer
/// @param _desc Tilemap descriptor
/// @return Tilemap id (or ID_NULL on error)
TilemapID CreateTilemap(TilemapList* _list, TilemapDesc _desc);

/// @brief Remove the tilemap from the list.
/// @param _list Tilemap list poitner
/// @param _id Tilemap id
void DestroyTilemap(TilemapList* _list, TilemapID _id);

/// @brief Get the tilemap scene grid square at the given worldspace position.
/// @param _list Tilemap list poitner
/// @param _id Tilemap id
/// @param _pos Worlspace position
/// @return Tilemap scene grid square
Vector2i GetTilemapGridFromPosition(TilemapList* _list, TilemapID _id, Vector2 _pos);

/// @brief Get the index of the tile at the given grid square in the base texture.
/// @param _list Tilemap list poitner
/// @param _id Tilemap id
/// @param _grid Tilemap texture grid sqaure
/// @return Tile index
TileIdx GetTileTextureIndex(TilemapList* _list, TilemapID _id, Vector2i _grid);

/// @brief Get the index of the tile at the given grid square in the map.
/// @param _list Tilemap list poitner
/// @param _id Tilemap id
/// @param _grid Tilemap scene grid square
/// @return Tile index
TileIdx GetTilemapIndex(TilemapList* _list, TilemapID _id, Vector2i _grid);

/// @brief Set the index of the tile at the given grid square in the map.
/// @param _list Tilemap list poitner
/// @param _id Tilemap id
/// @param _grid Tilemap scene grid square
void SetTilemapIndex(TilemapList* _list, TilemapID _id, Vector2i _grid, TileIdx _idx);

/// @brief Set the index of every tile using the given array of indices.
/// @param _list Tilemap list poitner
/// @param _id Tilemap id
/// @param _data Array of tile indices
/// @param _dataSize Size of array
void SetTilemapIndexAll(TilemapList* _list, TilemapID _id, TileIdx* _data, size_t _dataSize);

/// @brief Set the tilemaps draw depth.
/// @param _list Tilemap list pointer
/// @param _id Tilemap id
/// @param _depth Depth
void SetTilemapDepth(TilemapList* _list, TilemapID _id, int _depth);

/// @brief Get the tilemaps draw depth.
/// @param _list Tilemap list pointer
/// @param _id Tilemap id
/// @return Depth
int GetTilemapDepth(TilemapList* _list, TilemapID _id);

/// @brief Set whether or not the tilemap should be drawn.
/// @param _list Tilemap list pointer
/// @param _id Tilemap id
/// @param _visible If true, tilemap will be drawn
void SetTilemapVisible(TilemapList* _list, TilemapID _id, bool _visible);

/// @brief Get whether or not the tilemap should be drawn.
/// @param _list Tilemap list pointer
/// @param _id Tilemap id
/// @return If true, tilemap will be drawn
bool GetTilemapVisible(TilemapList* _list, TilemapID _id);

#endif