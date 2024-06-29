#include "luna/tile.h"

void _DrawTilemap(Tilemap* _tile) {
	if (!_tile) { return; }
	//LUNA_DBG_LOG("(_DrawTilemap) Drawing (%d) tiles", (int)vector_size(_tile->_data));
	for(size_t i=0; i<vector_size(_tile->_data); ++i) {
		// Get tile id
		unsigned int idx = *(unsigned int*)(vector_get(_tile->_data, i));
		//LUNA_DBG_LOG("(_DrawTilemap) Drawing tile index (%d)", (int)idx);
		if (idx != ID_NULL && _tile_valid_idx(_tile, idx)) {
			// Get position of tile id within base texture
			idx--;
			Vector2i texGrid = { idx % _tile->texMapSize.x, idx / _tile->texMapSize.x };
			Vector2i texSpacing = Vector2iAdd(_tile->texTileSize, _tile->texTileSpacing);
			Vector2i texPos = Vector2iAdd(_tile->texOffset, Vector2iMultiply(texGrid, texSpacing));

			// Get worldspace position
			Vector2i sceneGrid = { (int32_t)i % _tile->sceneMapSize.x, (int32_t)i / _tile->sceneMapSize.x };
			Vector2i scenePos = Vector2iAdd(_tile->sceneOffset, Vector2iMultiply(sceneGrid, _tile->texTileSize));

			// Draw texture
			Rectangle rec = {
				(float)texPos.x,
				(float)texPos.y,
				(float)_tile->texTileSize.x,
				(float)_tile->texTileSize.y,
			};
			DrawTextureRec(_tile->texture, rec, Vector2iCastFloat(scenePos), _tile->tint);
		}
	}
}

Tilemap* _CreateTilemap(TilemapDesc _desc) {
	Tilemap* tile = NULL;

	// Error check
	if (_desc.texTileSize.x == 0 || _desc.texTileSize.y == 0) { goto create_tilemap_fail; }

	// Create data array
	tile = calloc(1, sizeof *tile);
	if (!tile) { goto create_tilemap_fail; }
	tile->_data = vector_create_size(unsigned int, (_desc.sceneMapSize.x * _desc.sceneMapSize.y));
	if (!tile->_data) { goto create_tilemap_fail; }

	// Copy from descriptor
	tile->texture = _desc.texture;
	tile->tint = _desc.tint;
	tile->texOffset = _desc.texOffset;
	tile->sceneOffset = _desc.sceneOffset;
	tile->texTileSize = _desc.texTileSize;
	tile->texTileSpacing = _desc.texTileSpacing;
	tile->sceneMapSize = _desc.sceneMapSize;
	tile->texMapSize.x = (_desc.texture.width -  _desc.texOffset.x) / _desc.texTileSize.x;
	tile->texMapSize.y = (_desc.texture.height - _desc.texOffset.y) / _desc.texTileSize.y;
	return tile;

create_tilemap_fail:
	if (tile) { vector_destroy(tile->_data); }
	free(tile);
	return NULL;
}

void _DestroyTilemap(Tilemap* _tile) {
	if (_tile) {
		vector_destroy(_tile->_data);
		free(_tile);
	}
}

Vector2i GetTilemapGridFromPosition(Tilemap* _tile, Vector2 _pos) {
	Vector2i ret = { -1, -1 };
	if (!_tile) { return ret; }
	ret.x = (int32_t)_pos.x;
	ret.y = (int32_t)_pos.y;
	ret = Vector2iSubtract(ret, _tile->sceneOffset);
	ret = Vector2iDivide(ret, _tile->texTileSize);
	return ret;
}

unsigned int GetTileTextureIndex(Tilemap* _tile, Vector2i _grid) {
	if (!_tile) { return TILE_NULL; }
	return ((_grid.y * _tile->sceneMapSize.x) + _grid.x) + 1;
}

unsigned int GetTilemapIndex(Tilemap* _tile, Vector2i _grid) {
	if (!_tile) { return TILE_NULL; }
	if (!_tile_valid_grid(_tile, _grid)) { return TILE_NULL; }
	void* get = vector_get(_tile->_data, (_grid.y * _tile->sceneMapSize.x) + _grid.x);
	if (!get) { return TILE_NULL; }
	else { return *(unsigned int*)(get); }
}

int SetTilemapIndex(Tilemap* _tile, Vector2i _grid, unsigned int _idx) {
	if (!_tile) { return -1; }
	if (!_tile_valid_grid(_tile, _grid)) { return -2; }
	if (!_tile_valid_idx(_tile, _idx)) { return -3; }
	unsigned int _dataGrid = (_grid.y * _tile->sceneMapSize.x) + _grid.x;
	if (_dataGrid >= vector_size(_tile->_data)) { return -4; }
	vector_set(_tile->_data, _dataGrid, &_idx);
	return 0;
}

int SetTilemapIndexAll(Tilemap* _tile, unsigned int* _data, size_t _dataSize) {
	int ret = 0;
	vector_t* tmp_data = NULL;

	// Error check
	if (!_tile || !_data) { 
		ret = -1;
		goto set_tilemap_index_all_fail; 
	}
	if (_dataSize != (_tile->sceneMapSize.x * _tile->sceneMapSize.y)) { 
		ret = -2;
		goto set_tilemap_index_all_fail;
	}

	// Verify indices & copy into temp array
	tmp_data = vector_create_size(unsigned int, _dataSize);
	if (!tmp_data) { 
		ret = -3;
		goto set_tilemap_index_all_fail;
	}
	for(size_t i=0; i<_dataSize; ++i) {
		unsigned int idx = _data[i];
		if (!_tile_valid_idx(_tile, idx)) { 
			ret = -4;
			goto set_tilemap_index_all_fail;
		}
		vector_push_back(tmp_data, &idx);
	}

	// Overwrite tile map with temp array
	vector_destroy(_tile->_data);
	_tile->_data = tmp_data;
	return 0;
set_tilemap_index_all_fail:
	vector_destroy(tmp_data);
	return ret;
}