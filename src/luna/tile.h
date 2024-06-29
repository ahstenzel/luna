#ifndef LUNA_TILE_H
#define LUNA_TILE_H
/**
 * tile.h
 * Scene tile map object.
*/
#include "luna/common.h"

#define TILE_NULL 0

/// @brief Check if the given grid square is within the bounds of the scenes tile map.
/// @param t Tilemap object pointer
/// @param v Tilemap scene grid square
#define _tile_valid_grid(t, v) ((v).x >= 0 && (v).x < (t)->sceneMapSize.x && (v).y >= 0 && (v).y < (t)->sceneMapSize.y)

/// @brief Check if the given index is valid within the base texture of the tilemap.
/// @param t Tilemap object pointer
/// @param i Tile index
#define _tile_valid_idx(t, i) ((i) >= 0 && (int32_t)(i) < (((t)->texMapSize.x * (t)->texMapSize.y) + 1))

/// @brief Grid of tiles
typedef struct {
	Texture2D texture;			/* Base texture */
	Color tint;					/* Color tint */
	Vector2i texOffset;			/* Offset (in pixels) of first tile in the texture */
	Vector2i sceneOffset;		/* Offset (in pixels) of the tile map in the scene */
	Vector2i texTileSize;		/* Size (in pixels) of a single tile in the texture */
	Vector2i texTileSpacing;	/* Space (in pixels) between tiles in the texture */
	Vector2i sceneMapSize;		/* Size (in grid squares) of the tile map in the scene */
	Vector2i texMapSize;		/* Size (in grid squares) of the tile map in the texture */
	vector_t* _data;			/* Container of tile data */
} Tilemap;

/// @brief Descriptor for creating a tile object
typedef struct {
	Texture2D texture;			/* Base texture */
	Color tint;					/* Color tint */
	Vector2i texOffset;			/* Offset (in pixles) of first tile in the texture */
	Vector2i sceneOffset;		/* Offset (in pixels) of the tile map in the scene */
	Vector2i texTileSize;		/* Size (in pixels) of a single tile in the texture */
	Vector2i texTileSpacing;	/* Space (in pixels) between tiles in the texture */
	Vector2i sceneMapSize;		/* Size (in grid squares) of the tile map in the scene */
} TilemapDesc;

/// @brief Draw the tilemap.
/// @param _tile Tile object pointer
void _DrawTilemap(Tilemap* _tile);

/// @brief Create a new tilemap object. If an error occurred, _errno will be set to nonzero value.
/// @param _desc Tilemap descriptor
/// @return Tilemap object pointer, or NULL on failure
Tilemap* _CreateTilemap(TilemapDesc _desc);

/// @brief Deallocate the tilemap object.
/// @param _tile Tilemap object pointer
void _DestroyTilemap(Tilemap* _tile);

/// @brief Get the tilemap scene grid square at the given worldspace position.
/// @param _tile Tilemap object pointer
/// @param _pos Worlspace position
/// @return Tilemap scene grid square
Vector2i GetTilemapGridFromPosition(Tilemap* _tile, Vector2 _pos);

/// @brief Get the index of the tile at the given grid square in the base texture.
/// @param _tile Tilemap object pointer
/// @param _grid Tilemap texture grid sqaure
/// @return Tile index
unsigned int GetTileTextureIndex(Tilemap* _tile, Vector2i _grid);

/// @brief Get the index of the tile at the given grid square in the map.
/// @param _tile Tilemap object pointer
/// @param _grid Tilemap scene grid square
/// @return Tile index
unsigned int GetTilemapIndex(Tilemap* _tile, Vector2i _grid);

/// @brief Set the index of the tile at the given grid square in the map.
/// @param _tile Tilemap object pointer
/// @param _grid Tilemap scene grid square
/// @param _idx Return code (Zero if successful, nonzero otherwize)
int SetTilemapIndex(Tilemap* _tile, Vector2i _grid, unsigned int _idx);

/// @brief Set the index of every tile using the given array of indices.
/// @param _tile Tilemap object pointer
/// @param _data Array of tile indices
/// @param _dataSize Size of array
/// @return Return code (Zero if successful, nonzero otherwize)
int SetTilemapIndexAll(Tilemap* _tile, unsigned int* _data, size_t _dataSize);

#endif