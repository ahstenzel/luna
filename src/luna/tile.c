#include "luna/tile.h"

void _DrawTilemap(Tilemap* _tile) {
	if (!_tile) { 
		LUNA_DBG_WARN("Invalid tilemap reference!");
		return; 
	}
	for(size_t i=0; i<vector_size(_tile->_data); ++i) {
		// Get tile id
		unsigned int idx = *(unsigned int*)(vector_get(_tile->_data, i));
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

TilemapList* _CreateTilemapList(bool _depthSorting) {
	TilemapList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list!");
		return NULL; 
	}

	list->depthSorting = _depthSorting;
	list->tilemapIndices = unordered_map_create(size_t);
	list->tilemaps = free_list_create(Tilemap);
	list->tilemapDepthOrder = priority_queue_create(size_t);
	if (!list->tilemapIndices || !list->tilemaps || !list->tilemapDepthOrder) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list containers!");
		_DestroyTilemapList(list);
		return NULL;
	}
	return list;
}

void _DestroyTilemapList(TilemapList* _list) {
	if (_list) {
		unordered_map_destroy(_list->tilemapIndices);
		free_list_destroy(_list->tilemaps);
		priority_queue_destroy(_list->tilemapDepthOrder);
		free(_list);
	}
}

void _DrawTilemapList(TilemapList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return; 
	}

	if (_list->depthSorting) {
		for(priority_queue_it_t* it = priority_queue_it_begin(_list->tilemapDepthOrder); it; priority_queue_it_next(it)) {
			size_t idx = *(size_t*)(it->data);
			Tilemap* tilemap = free_list_get(_list->tilemaps, idx);
			if (!tilemap) {
				LUNA_DBG_WARN("Invalid tilemap reference!");
			}
			else if (tilemap->visible) {
				_DrawTilemap(tilemap);
			}
		}
	}
	else {
		for(free_list_it_t* it = free_list_it(_list->tilemaps); it; free_list_it_next(it)) {
			Tilemap* tilemap = it->data;
			if (!tilemap) {
				LUNA_DBG_WARN("Invalid tilemap reference!");
			}
			else if (tilemap->visible) {
				_DrawTilemap(tilemap);
			}
		}
	}
}

TilemapID CreateTilemap(TilemapList* _list, TilemapDesc _desc) {
	// Error check
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return ID_NULL; 
	}
	if (_desc.texTileSize.x == 0 || _desc.texTileSize.y == 0) { 
		LUNA_DBG_WARN("Invalid tilemap texture tile size, width or height must be nonzero!");
		return ID_NULL; 
	}

	// Create data array
	TilemapID id = _luna_id_generate();
	Tilemap tile = {
		.texture = _desc.texture,
		.tint = _desc.tint,
		.texOffset = _desc.texOffset,
		.sceneOffset = _desc.sceneOffset,
		.texTileSize = _desc.texTileSize,
		.texTileSpacing = _desc.texTileSpacing,
		.sceneMapSize = _desc.sceneMapSize,
		.texMapSize = {
			(_desc.texture.width -  _desc.texOffset.x) / _desc.texTileSize.x, 
			(_desc.texture.height - _desc.texOffset.y) / _desc.texTileSize.y 
		},
		._data = vector_create_size(TileIdx, (_desc.sceneMapSize.x * _desc.sceneMapSize.y)),
		.visible = _desc.visible,
		.depth = _desc.depth
	};
	if (!tile._data) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap data buffer!");
		return ID_NULL; 
	}

	// Add to list
	size_t idx = 0;
	if (!free_list_insert(_list->tilemaps, &idx, &tile)) {
		LUNA_DBG_WARN("Failed to add tilemap to list!");
		vector_destroy(tile._data);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->tilemapIndices, id, &idx)) {
		LUNA_DBG_WARN("Failed to add tilemap to index map!");
		vector_destroy(tile._data);
		free_list_remove(_list->tilemaps, idx);
		return ID_NULL;
	}
	if (_list->depthSorting) {
		if (!priority_queue_push(_list->tilemapDepthOrder, tile.depth, &idx)) {
			LUNA_DBG_WARN("Failed to add tilemap to depth queue!");
			vector_destroy(tile._data);
			free_list_remove(_list->tilemaps, idx);
			unordered_map_delete(_list->tilemapIndices, id);
			return ID_NULL;
		}
	}
	return id;
}

void DestroyTilemap(TilemapList* _list, TilemapID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		if (_list->depthSorting) {
			Tilemap* tile = free_list_get(_list->tilemaps, *idxPtr);
			if (tile) {
				priority_queue_remove(_list->tilemapDepthOrder, tile->depth, idxPtr);
			}
		}
		free_list_remove(_list->tilemaps, *idxPtr);
		unordered_map_delete(_list->tilemapIndices, _id);
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
}

Vector2i GetTilemapGridFromPosition(TilemapList* _list, TilemapID _id, Vector2 _pos) {
	Vector2i ret = { -1, -1 };
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return ret; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Get property
		ret.x = (int32_t)_pos.x;
		ret.y = (int32_t)_pos.y;
		ret = Vector2iSubtract(ret, tilemap->sceneOffset);
		ret = Vector2iDivide(ret, tilemap->texTileSize);
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return ret;
}

TileIdx GetTileTextureIndex(TilemapList* _list, TilemapID _id, Vector2i _grid) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return TILE_NULL; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Get property
		return ((_grid.y * tilemap->sceneMapSize.x) + _grid.x) + 1;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return TILE_NULL;
}

TileIdx GetTilemapIndex(TilemapList* _list, TilemapID _id, Vector2i _grid) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return TILE_NULL; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Get property
		if (!_tile_valid_grid(tilemap, _grid)) { return TILE_NULL; }
		void* get = vector_get(tilemap->_data, (_grid.y * tilemap->sceneMapSize.x) + _grid.x);
		if (!get) { return TILE_NULL; }
		else { return *(TileIdx*)(get); }
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return TILE_NULL;
}

void SetTilemapIndex(TilemapList* _list, TilemapID _id, Vector2i _grid, TileIdx _idx) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Set property
		if (!_tile_valid_grid(tilemap, _grid)) { 
			LUNA_DBG_WARN("Invalid tile position! pos: (%d, %d), tilemap: (%d)", (int)_grid.x, (int)_grid.y, (int)_id);
			return; 
		}
		if (!_tile_valid_idx(tilemap, _idx)) { 
			LUNA_DBG_WARN("Invalid tile index! idx: (%d), tilemap: (%d)", (int)_idx, (int)_id);
			return; 
		}
		uint32_t _dataGrid = (_grid.y * tilemap->sceneMapSize.x) + _grid.x;
		if (_dataGrid >= vector_size(tilemap->_data)) { 
			LUNA_DBG_WARN("Undersized tilemap data buffer! tilemap: (%d)", (int)_id);
			return; 
		}
		vector_set(tilemap->_data, _dataGrid, &_idx);
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return;
}

void SetTilemapIndexAll(TilemapList* _list, TilemapID _id, TileIdx* _data, size_t _dataSize) {
	vector_t* tmp_data = NULL;

	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		goto set_tilemap_index_all_fail; 
	}
	if (!_data) {
		LUNA_DBG_WARN("Invalid tilemap data buffer!");
		goto set_tilemap_index_all_fail; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Error check
		if (_dataSize != (tilemap->sceneMapSize.x * tilemap->sceneMapSize.y)) { 
			LUNA_DBG_WARN("Tilemap data buffer size mismatch!");
			goto set_tilemap_index_all_fail;
		}

		// Verify indices & copy into temp array
		tmp_data = vector_create_size(TileIdx, _dataSize);
		if (!tmp_data) { 
			LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap temp data buffer!");
			goto set_tilemap_index_all_fail;
		}
		for(size_t i=0; i<_dataSize; ++i) {
			unsigned int idx = _data[i];
			vector_push_back(tmp_data, &idx);
		}

		// Overwrite tile map with temp array
		vector_destroy(tilemap->_data);
		tilemap->_data = tmp_data;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return;

set_tilemap_index_all_fail:
	vector_destroy(tmp_data);
}

void SetTilemapDepth(TilemapList* _list, TilemapID _id, int _depth) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Update depth & sort if necessary
		if (_list->depthSorting) {
			priority_queue_remove(_list->tilemapDepthOrder, tilemap->depth, idxPtr);
			priority_queue_push(_list->tilemapDepthOrder, _depth, idxPtr);
		}
		tilemap->depth = _depth;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
}

int GetTilemapDepth(TilemapList* _list, TilemapID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return 0; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Get property
		return tilemap->depth;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return 0;
}

void SetTilemapVisible(TilemapList* _list, TilemapID _id, bool _visible) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Get property
		tilemap->visible = _visible;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
}

bool GetTilemapVisible(TilemapList* _list, TilemapID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return false; 
	}

	size_t* idxPtr = unordered_map_find(_list->tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->tilemaps, *idxPtr);

		// Get property
		return tilemap->visible;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return false;
}