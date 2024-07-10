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
			Vector2i texGrid = { idx % _tile->_texMapSize.x, idx / _tile->_texMapSize.x };
			Vector2i texSpacing = Vector2iAdd(_tile->_texTileSize, _tile->_texTileSpacing);
			Vector2i texPos = Vector2iAdd(_tile->_texOffset, Vector2iMultiply(texGrid, texSpacing));

			// Get worldspace position
			Vector2i sceneGrid = { (int32_t)i % _tile->_sceneMapSize.x, (int32_t)i / _tile->_sceneMapSize.x };
			Vector2i scenePos = Vector2iAdd(_tile->_sceneOffset, Vector2iMultiply(sceneGrid, _tile->_texTileSize));

			// Draw texture
			Rectangle rec = {
				(float)texPos.x,
				(float)texPos.y,
				(float)_tile->_texTileSize.x,
				(float)_tile->_texTileSize.y,
			};
			DrawTextureRec(_tile->_texture, rec, Vector2iCastFloat(scenePos), _tile->_tint);
		}
	}
}

TilemapList* _CreateTilemapList(bool _depthSorting) {
	TilemapList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list!");
		return NULL; 
	}

	list->_depthSorting = _depthSorting;
	list->_tilemapIndices = unordered_map_create(size_t);
	list->_tilemaps = free_list_create(Tilemap);
	list->_tilemapDepthOrder = priority_queue_create(size_t);
	if (!list->_tilemapIndices || !list->_tilemaps || !list->_tilemapDepthOrder) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list containers!");
		_DestroyTilemapList(list);
		return NULL;
	}
	return list;
}

void _DestroyTilemapList(TilemapList* _list) {
	if (_list) {
		unordered_map_destroy(_list->_tilemapIndices);
		free_list_destroy(_list->_tilemaps);
		priority_queue_destroy(_list->_tilemapDepthOrder);
		free(_list);
	}
}

void _DrawTilemapList(TilemapList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return; 
	}

	if (_list->_depthSorting) {
		for(priority_queue_it_t* it = priority_queue_it_begin(_list->_tilemapDepthOrder); it; priority_queue_it_next(it)) {
			size_t idx = *(size_t*)(it->data);
			Tilemap* tilemap = free_list_get(_list->_tilemaps, idx);
			if (!tilemap) {
				LUNA_DBG_WARN("Invalid tilemap reference!");
			}
			else if (tilemap->_visible) {
				_DrawTilemap(tilemap);
			}
		}
	}
	else {
		for(free_list_it_t* it = free_list_it(_list->_tilemaps); it; free_list_it_next(it)) {
			Tilemap* tilemap = it->data;
			if (!tilemap) {
				LUNA_DBG_WARN("Invalid tilemap reference!");
			}
			else if (tilemap->_visible) {
				_DrawTilemap(tilemap);
			}
		}
	}
}

size_t GetTilemapSize(TilemapList* _list) {
	if (!_list) {
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		return 0; 
	}
	return free_list_size(_list->_tilemaps);
}

TilemapListIt* TilemapListItBegin(TilemapList* _list) {
	// Error check
	TilemapListIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid sprite list reference!");
		goto tilemap_list_it_begin_fail;
	}
	if (free_list_size(_list->_tilemaps) == 0) { 
		goto tilemap_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list iterator!");
		goto tilemap_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->_tilemapIndices);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list index map iterator!");
		goto tilemap_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Tilemap list index map iterator points to invalid data!");
		goto tilemap_list_it_begin_fail; 
	}

	// Retrieve tilemap data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_tilemaps, *idx);
	it->id = (TilemapID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DBG_WARN("Tilemap list iterator points to invalid data!");
		goto tilemap_list_it_begin_fail; 
	}

	return it;
tilemap_list_it_begin_fail:
	free(it);
	return NULL;
}

void TilemapListItNext(TilemapListIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	TilemapListIt* it = *_it;
	unordered_map_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->_tilemaps, *idx);
		it->id = (TilemapID)it->_ptr->key;
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid tilemap data!");
			free(it);
			(*_it) = NULL;
			return;
		}
	}
	else {
		// Deallocate iterator
		free(it);
		(*_it) = NULL;
		return;
	}
}

TilemapListDepthIt* TilemapListDepthItBegin(TilemapList* _list) {
	// Error check
	TilemapListDepthIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		goto tilemap_list_depth_it_begin_fail;
	}
	if (free_list_size(_list->_tilemaps) == 0) { 
		goto tilemap_list_depth_it_begin_fail; 
	}

	// Create queue iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list iterator!");
		goto tilemap_list_depth_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = priority_queue_it_begin(_list->_tilemapDepthOrder);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list depth queue iterator!");
		goto tilemap_list_depth_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Tilemap list depth queue iterator points to invalid data!");
		goto tilemap_list_depth_it_begin_fail; 
	}

	// Retrieve tilemap data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_tilemaps, *idx);
	if (!it->data) { 
		LUNA_DBG_WARN("Tilemap list iterator points to invalid data!");
		goto tilemap_list_depth_it_begin_fail; 
	}
	it->id = (TilemapID)it->data->_id;

	return it;
tilemap_list_depth_it_begin_fail:
	free(it);
	return NULL;
}

TilemapListDepthIt* TilemapListDepthItRBegin(TilemapList* _list) {
		// Error check
	TilemapListDepthIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid tilemap list reference!");
		goto tilemap_list_depth_it_begin_fail;
	}
	if (free_list_size(_list->_tilemaps) == 0) { 
		goto tilemap_list_depth_it_begin_fail; 
	}

	// Create queue iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list iterator!");
		goto tilemap_list_depth_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = priority_queue_it_rbegin(_list->_tilemapDepthOrder);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap list depth queue iterator!");
		goto tilemap_list_depth_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Tilemap list depth queue iterator points to invalid data!");
		goto tilemap_list_depth_it_begin_fail; 
	}

	// Retrieve tilemap data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_tilemaps, *idx);
	if (!it->data) { 
		LUNA_DBG_WARN("Tilemap list iterator points to invalid data!");
		goto tilemap_list_depth_it_begin_fail; 
	}
	it->id = (TilemapID)it->data->_id;

	return it;
tilemap_list_depth_it_begin_fail:
	free(it);
	return NULL;
}

void TilemapListDepthItNext(TilemapListDepthIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	TilemapListDepthIt* it = *_it;
	priority_queue_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->_tilemaps, *idx);
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid tilemap data!");
			free(it);
			(*_it) = NULL;
			return;
		}
		it->id = (TilemapID)it->data->_id;
	}
	else {
		// Deallocate iterator
		free(it);
		(*_it) = NULL;
		return;
	}
}

void TilemapListDepthItPrev(TilemapListDepthIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	TilemapListDepthIt* it = *_it;
	priority_queue_it_prev(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->_tilemaps, *idx);
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid tilemap data!");
			free(it);
			(*_it) = NULL;
			return;
		}
		it->id = (TilemapID)it->data->_id;
	}
	else {
		// Deallocate iterator
		free(it);
		(*_it) = NULL;
		return;
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
		._texture = _desc.texture,
		._tint = _desc.tint,
		._texOffset = _desc.texOffset,
		._sceneOffset = _desc.sceneOffset,
		._texTileSize = _desc.texTileSize,
		._texTileSpacing = _desc.texTileSpacing,
		._sceneMapSize = _desc.sceneMapSize,
		._texMapSize = {
			(_desc.texture.width -  _desc.texOffset.x) / _desc.texTileSize.x, 
			(_desc.texture.height - _desc.texOffset.y) / _desc.texTileSize.y 
		},
		._data = vector_create_size(TileIdx, (_desc.sceneMapSize.x * _desc.sceneMapSize.y)),
		._visible = _desc.visible,
		._depth = _desc.depth
	};
	if (!tile._data) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate tilemap data buffer!");
		return ID_NULL; 
	}

	// Add to list
	size_t idx = 0;
	if (!free_list_insert(_list->_tilemaps, &idx, &tile)) {
		LUNA_DBG_WARN("Failed to add tilemap to list!");
		vector_destroy(tile._data);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->_tilemapIndices, id, &idx)) {
		LUNA_DBG_WARN("Failed to add tilemap to index map!");
		vector_destroy(tile._data);
		free_list_remove(_list->_tilemaps, idx);
		return ID_NULL;
	}
	if (_list->_depthSorting) {
		if (!priority_queue_push(_list->_tilemapDepthOrder, tile._depth, &idx)) {
			LUNA_DBG_WARN("Failed to add tilemap to depth queue!");
			vector_destroy(tile._data);
			free_list_remove(_list->_tilemaps, idx);
			unordered_map_delete(_list->_tilemapIndices, id);
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		if (_list->_depthSorting) {
			Tilemap* tile = free_list_get(_list->_tilemaps, *idxPtr);
			if (tile) {
				priority_queue_remove(_list->_tilemapDepthOrder, tile->_depth, idxPtr);
			}
		}
		free_list_remove(_list->_tilemaps, *idxPtr);
		unordered_map_delete(_list->_tilemapIndices, _id);
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Get property
		ret.x = (int32_t)_pos.x;
		ret.y = (int32_t)_pos.y;
		ret = Vector2iSubtract(ret, tilemap->_sceneOffset);
		ret = Vector2iDivide(ret, tilemap->_texTileSize);
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Get property
		return ((_grid.y * tilemap->_sceneMapSize.x) + _grid.x) + 1;
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Get property
		if (!_tile_valid_grid(tilemap, _grid)) { return TILE_NULL; }
		void* get = vector_get(tilemap->_data, (_grid.y * tilemap->_sceneMapSize.x) + _grid.x);
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Set property
		if (!_tile_valid_grid(tilemap, _grid)) { 
			LUNA_DBG_WARN("Invalid tile position! pos: (%d, %d), tilemap: (%d)", (int)_grid.x, (int)_grid.y, (int)_id);
			return; 
		}
		if (!_tile_valid_idx(tilemap, _idx)) { 
			LUNA_DBG_WARN("Invalid tile index! idx: (%d), tilemap: (%d)", (int)_idx, (int)_id);
			return; 
		}
		uint32_t _dataGrid = (_grid.y * tilemap->_sceneMapSize.x) + _grid.x;
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Error check
		if (_dataSize != (tilemap->_sceneMapSize.x * tilemap->_sceneMapSize.y)) { 
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Update depth & sort if necessary
		if (_list->_depthSorting) {
			priority_queue_remove(_list->_tilemapDepthOrder, tilemap->_depth, idxPtr);
			priority_queue_push(_list->_tilemapDepthOrder, _depth, idxPtr);
		}
		tilemap->_depth = _depth;
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Get property
		return tilemap->_depth;
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Get property
		tilemap->_visible = _visible;
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

	size_t* idxPtr = unordered_map_find(_list->_tilemapIndices, _id);
	if (idxPtr) {
		Tilemap* tilemap = free_list_get(_list->_tilemaps, *idxPtr);

		// Get property
		return tilemap->_visible;
	}
	else {
		LUNA_DBG_WARN("Invalid tilemap id (%d)!", (int)_id);
	}
	return false;
}