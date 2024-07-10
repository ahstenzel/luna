#include "luna/sprite.h"

void _UpdateSprite(Sprite* _sprite, float _dt) {
	if (!_sprite) { 
		LUNA_DBG_WARN("Invalid sprite reference!");
		return; 
	}

	_sprite->_timer = fmodf(_sprite->_timer + _dt, _sprite->imageSpeed);
	if (_sprite->_timer < _dt) {
		_sprite->imageIndex = (_sprite->imageIndex + 1) % _sprite->imageNum;
	}
}

void _DrawSprite(Sprite* _sprite) {
	if (!_sprite) { 
		LUNA_DBG_WARN("Invalid sprite reference!");
		return; 
	}

	float width = _sprite->texture.width / (float)_sprite->numCols;
	float height = _sprite->texture.height / (float)_sprite->numRows;
	size_t col = _sprite->imageIndex % _sprite->numCols;
	size_t row = _sprite->imageIndex / _sprite->numCols;
	Rectangle source = {
		width*col,
		height*row,
		width,
		height
	};
	Rectangle dest = {
		_sprite->position.x,
		_sprite->position.y,
		width * _sprite->scale.x,
		height * _sprite->scale.y
	};
	Vector2 origin = {_sprite->origin.x * width * _sprite->scale.x, _sprite->origin.y * height * _sprite->scale.y};
	DrawTexturePro(_sprite->texture, source, dest, origin, _sprite->rotation, _sprite->tint);
}

SpriteList* _CreateSpriteList(bool _depthSorting) {
	SpriteList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list!");
		return NULL; 
	}

	list->depthSorting = _depthSorting;
	list->spriteIndices = unordered_map_create(size_t);
	list->sprites = free_list_create(Sprite);
	list->spriteDepthOrder = priority_queue_create(size_t);
	if (!list->spriteIndices || !list->sprites || !list->spriteDepthOrder) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list containers!");
		_DestroySpriteList(list);
		return NULL;
	}
	return list;
}

void _DestroySpriteList(SpriteList* _list) {
	if (_list) {
		unordered_map_destroy(_list->spriteIndices);
		free_list_destroy(_list->sprites);
		priority_queue_destroy(_list->spriteDepthOrder);
		free(_list);
	}
}

void _UpdateSpriteList(SpriteList* _list, float _dt) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return; 
	}

	for(free_list_it_t* it = free_list_it(_list->sprites); it; free_list_it_next(it)) {
		Sprite* sprite = it->data;
		_UpdateSprite(sprite, _dt);
	}
}

void _DrawSpriteList(SpriteList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return; 
	}

	if (_list->depthSorting) {
		for(priority_queue_it_t* it = priority_queue_it_begin(_list->spriteDepthOrder); it; priority_queue_it_next(it)) {
			size_t idx = *(size_t*)(it->data);
			Sprite* sprite = free_list_get(_list->sprites, idx);
			if (!sprite) {
				LUNA_DBG_WARN("Invalid sprite reference!");
			}
			else if (sprite->visible) {
				_DrawSprite(sprite);
			}
		}
	}
	else {
		for(free_list_it_t* it = free_list_it(_list->sprites); it; free_list_it_next(it)) {
			Sprite* sprite = it->data;
			if (!sprite) {
				LUNA_DBG_WARN("Invalid sprite reference!");
			}
			else if (sprite->visible) {
				_DrawSprite(sprite);
			}
		}
	}
}

size_t GetSpriteListSize(SpriteList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return 0; 
	}
	return free_list_size(_list->sprites);
}

SpriteListIt* SpriteListItBegin(SpriteList* _list) {
	// Error check
	SpriteListIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid sprite list reference!");
		goto sprite_list_it_begin_fail;
	}
	if (free_list_size(_list->sprites) == 0) { 
		goto sprite_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list iterator!");
		goto sprite_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->spriteIndices);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list index map iterator!");
		goto sprite_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Sprite list index map iterator points to invalid data!");
		goto sprite_list_it_begin_fail; 
	}

	// Retrieve sprite data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->sprites, *idx);
	it->id = (SpriteID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DBG_WARN("Sprite list iterator points to invalid data!");
		goto sprite_list_it_begin_fail; 
	}

	return it;
sprite_list_it_begin_fail:
	free(it);
	return NULL;
}

void SpriteListItNext(SpriteListIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	SpriteListIt* it = *_it;
	unordered_map_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->sprites, *idx);
		it->id = (SpriteID)it->_ptr->key;
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid sprite data!");
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

SpriteListDepthIt* SpriteListDepthItBegin(SpriteList* _list) {
	// Error check
	SpriteListDepthIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid sprite list reference!");
		goto sprite_list_depth_it_begin_fail;
	}
	if (free_list_size(_list->sprites) == 0) { 
		goto sprite_list_depth_it_begin_fail; 
	}

	// Create queue iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list iterator!");
		goto sprite_list_depth_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = priority_queue_it_begin(_list->spriteDepthOrder);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list depth queue iterator!");
		goto sprite_list_depth_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Sprite list depth queue iterator points to invalid data!");
		goto sprite_list_depth_it_begin_fail; 
	}

	// Retrieve sprite data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->sprites, *idx);
	if (!it->data) { 
		LUNA_DBG_WARN("Sprite list iterator points to invalid data!");
		goto sprite_list_depth_it_begin_fail; 
	}
	it->id = (SpriteID)it->data->id;

	return it;
sprite_list_depth_it_begin_fail:
	free(it);
	return NULL;
}

SpriteListDepthIt* SpriteListDepthItRBegin(SpriteList* _list) {
	// Error check
	SpriteListDepthIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid sprite list reference!");
		goto sprite_list_depth_rbegin_fail;
	}
	if (free_list_size(_list->sprites) == 0) { 
		goto sprite_list_depth_rbegin_fail; 
	}

	// Create queue iterator
	it = calloc(1, sizeof *it);
	if (!it) { goto sprite_list_depth_rbegin_fail; }
	it->_list = _list;
	it->_ptr = priority_queue_it_rbegin(_list->spriteDepthOrder);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate sprite list depth queue iterator!");
		goto sprite_list_depth_rbegin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Sprite list depth queue iterator points to invalid data!");
		goto sprite_list_depth_rbegin_fail; 
	}

	// Retrieve sprite data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->sprites, *idx);
	if (!it->data) { 
		LUNA_DBG_WARN("Sprite list iterator points to invalid data!");
		goto sprite_list_depth_rbegin_fail; 
	}
	it->id = (SpriteID)it->data->id;

	return it;
sprite_list_depth_rbegin_fail:
	free(it);
	return NULL;
}

void SpriteListDepthItNext(SpriteListDepthIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	SpriteListDepthIt* it = *_it;
	priority_queue_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->sprites, *idx);
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid sprite data!");
			free(it);
			(*_it) = NULL;
			return;
		}
		it->id = (SpriteID)it->data->id;
	}
	else {
		// Deallocate iterator
		free(it);
		(*_it) = NULL;
		return;
	}
}

void SpriteListDepthItPrev(SpriteListDepthIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	SpriteListDepthIt* it = *_it;
	priority_queue_it_prev(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		size_t* idx = it->_ptr->data;
		it->data = free_list_get(it->_list->sprites, *idx);
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid sprite data!");
			free(it);
			(*_it) = NULL;
			return;
		}
		it->id = (SpriteID)it->data->id;
	}
	else {
		// Deallocate iterator
		free(it);
		(*_it) = NULL;
		return;
	}
}

SpriteID CreateSprite(SpriteList* _list, SpriteDesc _desc) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return ID_NULL; 
	}

	SpriteID id = _luna_id_generate();
	Sprite spr = {
		.texture = _desc.texture,
		.position = _desc.position,
		.scale = _desc.scale,
		.origin = _desc.origin,
		.tint = _desc.tint,
		.id = id,
		.depth = _desc.depth,
		.imageIndex = _desc.imageIndex,
		.imageNum = _desc.imageNum,
		.numRows = _desc.numRows,
		.numCols = _desc.numCols,
		.imageSpeed = _desc.imageSpeed,
		.rotation = _desc.rotation,
		._timer = 0.f,
		.visible = _desc.visible
	};
	size_t idx = 0;
	if (!free_list_insert(_list->sprites, &idx, &spr)) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to add sprite to list!");
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->spriteIndices, id, &idx)) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to add sprite to index map!");
		free_list_remove(_list->sprites, idx);
		return ID_NULL;
	}
	if (_list->depthSorting) {
		if (!priority_queue_push(_list->spriteDepthOrder, spr.depth, &idx)) {
			LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to add sprite to depth queue!");
			free_list_remove(_list->sprites, idx);
			unordered_map_delete(_list->spriteIndices, id);
			return ID_NULL;
		}
	}
	return id;
}

void DestroySprite(SpriteList* _list, SpriteID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return;
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		if (_list->depthSorting) {
			Sprite* spr = free_list_get(_list->sprites, *idxPtr);
			if (spr) {
				priority_queue_remove(_list->spriteDepthOrder, spr->depth, idxPtr);
			}
		}
		free_list_remove(_list->sprites, *idxPtr);
		unordered_map_delete(_list->spriteIndices, _id);
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
}

void SetSpritePosition(SpriteList* _list, SpriteID _id, Vector2 _position) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->position = _position;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
}

Vector2 GetSpritePosition(SpriteList* _list, SpriteID _id) {
	Vector2 ret = { 0.f, 0.f };
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return ret; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		ret.x = sprite->position.x;
		ret.y = sprite->position.y;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
	return ret;
}

void SetSpriteDepth(SpriteList* _list, SpriteID _id, int _depth) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return;
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update depth & sort if necessary
		if (_list->depthSorting) {
			priority_queue_remove(_list->spriteDepthOrder, sprite->depth, idxPtr);
			priority_queue_push(_list->spriteDepthOrder, _depth, idxPtr);
		}
		sprite->depth = _depth;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
}

int GetSpriteDepth(SpriteList* _list, SpriteID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return 0; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->depth;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
	return 0;
}

void SetSpriteImageIndex(SpriteList* _list, SpriteID _id, int _imageIndex) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->imageIndex = _imageIndex;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
}

int GetSpriteImageIndex(SpriteList* _list, SpriteID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return 0; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->imageIndex;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
	return 0;
}

void SetSpriteImageSpeed(SpriteList* _list, SpriteID _id, float _imageSpeed) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->imageSpeed = _imageSpeed;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
}

float GetSpriteImageSpeed(SpriteList* _list, SpriteID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return 0.f; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->imageSpeed;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
	return 0.f;
}

void SetSpriteVisible(SpriteList* _list, SpriteID _id, bool _visible) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->visible = _visible;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
}

bool GetSpriteVisible(SpriteList* _list, SpriteID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid sprite list reference!");
		return false; 
	}

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->visible;
	}
	else {
		LUNA_DBG_WARN("Invalid sprite id (%d)!", (int)_id);
	}
	return false;
}