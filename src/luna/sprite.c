#include "luna/sprite.h"

void _UpdateSprite(Sprite* _sprite, float _dt) {
	if (!_sprite) { 
		LUNA_DEBUG_WARN("Invalid sprite reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	// Update animation
	_sprite->_timer = fmodf(_sprite->_timer + _dt, 1.f / _sprite->_imageSpeed);
	if (_sprite->_timer < _dt) {
		_sprite->_imageIndex = (_sprite->_imageIndex + 1) % _sprite->_imageNum;
	}

	// Update rotation
	_sprite->_rotation += (Angle)(_dt * (float)_sprite->_rotationSpeed);

	// Update scrolling
	_sprite->_scrollTimer.x = fmodf(_sprite->_scrollTimer.x + _dt, 1.f / _sprite->_scrollSpeed.x);
	if (_sprite->_scrollTimer.x < _dt) {
		_sprite->_scrollOffset.x = fmodf((_sprite->_scrollOffset.x + 1.f), _sprite->_texture.width / (float)_sprite->_numCols);
	}
	_sprite->_scrollTimer.y = fmodf(_sprite->_scrollTimer.y + _dt, 1.f / _sprite->_scrollSpeed.y);
	if (_sprite->_scrollTimer.y < _dt) {
		_sprite->_scrollOffset.y = fmodf((_sprite->_scrollOffset.y + 1.f), _sprite->_texture.height / (float)_sprite->_numRows);
	}
}

void _DrawSprite(Sprite* _sprite) {
	if (!_sprite) { 
		LUNA_DEBUG_WARN("Invalid sprite reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	float width = _sprite->_texture.width / (float)_sprite->_numCols;
	float height = _sprite->_texture.height / (float)_sprite->_numRows;
	size_t col = _sprite->_imageIndex % _sprite->_numCols;
	size_t row = _sprite->_imageIndex / _sprite->_numCols;
	Vector2 origin = {
		_sprite->_origin.x * width * _sprite->_scale.x, 
		_sprite->_origin.y * height * _sprite->_scale.y
	};

	if (VECTOR2_IS_ZERO(_sprite->_scrollOffset)) {
		// Draw the entire sprite once
		Rectangle source = {
			width*col,
			height*row,
			width,
			height
		};
		Rectangle dest = {
			_sprite->_position.x,
			_sprite->_position.y,
			width * _sprite->_scale.x,
			height * _sprite->_scale.y
		};
		DrawTexturePro(_sprite->_texture, source, dest, origin, ANGLE_TO_DEG(_sprite->_rotation), _sprite->_tint);
	}
	else {
		// Draw portions of the sprite up to 4 times
		float scrollX = _sprite->_scrollOffset.x;
		float scrollY = _sprite->_scrollOffset.y;
		Rectangle s1 = { 
			.x = width*col,
			.y = height*row,
			.width = width - scrollX,
			.height =height - scrollY
		};
		Rectangle s2 = {
			.x = (width*col) + (width - scrollX),
			.y = height*row,
			.width = scrollX,
			.height = height - scrollY
		};
		Rectangle s3 = { 
			.x = width*col,
			.y = (height*row) + (height - scrollY),
			.width = width - scrollX,
			.height = scrollY
		};
		Rectangle s4 = { 
			.x = (width*col) + (width - scrollX),
			.y = (height*row) + (height - scrollY),
			.width = scrollX,
			.height = scrollY
		};
		Rectangle d1 = { 
			.x = _sprite->_position.x + (scrollX * _sprite->_scale.x),
			.y = _sprite->_position.y + (scrollY * _sprite->_scale.y),
			.width = s1.width * _sprite->_scale.x,
			.height = s1.height * _sprite->_scale.y
		};
		Rectangle d2 = { 
			.x = _sprite->_position.x,
			.y = _sprite->_position.y + (scrollY * _sprite->_scale.y),
			.width = s2.width * _sprite->_scale.x,
			.height = s2.height * _sprite->_scale.y
		};
		Rectangle d3 = { 
			.x = _sprite->_position.x + (scrollX * _sprite->_scale.x),
			.y = _sprite->_position.y,
			.width = s3.width * _sprite->_scale.x,
			.height = s3.height * _sprite->_scale.y
		};
		Rectangle d4 = { 
			.x = _sprite->_position.x,
			.y = _sprite->_position.y,
			.width = s4.width * _sprite->_scale.x,
			.height = s4.height * _sprite->_scale.y
		};

		if (s1.width != 0.f && s1.height != 0.f) {
			DrawTexturePro(_sprite->_texture, s1, d1, origin, ANGLE_TO_DEG(_sprite->_rotation), _sprite->_tint);
		}
		if (s2.width != 0.f && s2.height != 0.f) {
			DrawTexturePro(_sprite->_texture, s2, d2, origin, ANGLE_TO_DEG(_sprite->_rotation), _sprite->_tint);
		}
		if (s3.width != 0.f && s3.height != 0.f) {
			DrawTexturePro(_sprite->_texture, s3, d3, origin, ANGLE_TO_DEG(_sprite->_rotation), _sprite->_tint);
		}
		if (s4.width != 0.f && s4.height != 0.f) {
			DrawTexturePro(_sprite->_texture, s4, d4, origin, ANGLE_TO_DEG(_sprite->_rotation), _sprite->_tint);
		}
	}
	
}

SpriteList* _CreateSpriteList(bool _depthSorting) {
	SpriteList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list!");
		return NULL; 
	}

	list->_depthSorting = _depthSorting;
	list->_spriteIndices = unordered_map_create(size_t);
	list->_sprites = free_list_create(Sprite);
	list->_spriteDepthOrder = priority_queue_create(size_t);
	if (!list->_spriteIndices || !list->_sprites || !list->_spriteDepthOrder) {
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list containers!");
		_DestroySpriteList(list);
		return NULL;
	}
	return list;
}

void _DestroySpriteList(SpriteList* _list) {
	if (_list) {
		unordered_map_destroy(_list->_spriteIndices);
		free_list_destroy(_list->_sprites);
		priority_queue_destroy(_list->_spriteDepthOrder);
		free(_list);
	}
}

void _UpdateSpriteList(SpriteList* _list, float _dt) {
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	for(free_list_it_t* it = free_list_it(_list->_sprites); it; free_list_it_next(it)) {
		Sprite* sprite = it->data;
		_UpdateSprite(sprite, _dt);
	}
}

void _DrawSpriteList(SpriteList* _list) {
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	if (_list->_depthSorting) {
		for(priority_queue_it_t* it = priority_queue_it_begin(_list->_spriteDepthOrder); it; priority_queue_it_next(it)) {
			size_t idx = *(size_t*)(it->data);
			Sprite* sprite = free_list_get(_list->_sprites, idx);
			if (!sprite) {
				LUNA_DEBUG_WARN("Sprite list iterator points to invalid data!");
				LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			}
			else if (sprite->_visible) {
				_DrawSprite(sprite);
			}
		}
	}
	else {
		for(free_list_it_t* it = free_list_it(_list->_sprites); it; free_list_it_next(it)) {
			Sprite* sprite = it->data;
			if (!sprite) {
				LUNA_DEBUG_WARN("Sprite list iterator points to invalid data!");
				LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			}
			else if (sprite->_visible) {
				_DrawSprite(sprite);
			}
		}
	}
}

size_t GetSpriteListSize(SpriteList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0; 
	}
	return free_list_size(_list->_sprites);
}

SpriteListIt* SpriteListItBegin(SpriteList* _list) {
	LUNA_RETURN_CLEAR;
	// Error check
	SpriteListIt* it = NULL;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto sprite_list_it_begin_fail;
	}
	if (free_list_size(_list->_sprites) == 0) { 
		goto sprite_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list iterator!");
		goto sprite_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->_spriteIndices);
	if (!it->_ptr) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list index map iterator!");
		goto sprite_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DEBUG_WARN("Sprite list index map iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto sprite_list_it_begin_fail; 
	}

	// Retrieve sprite data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_sprites, *idx);
	it->id = (SpriteID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DEBUG_WARN("Sprite list iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto sprite_list_it_begin_fail; 
	}

	return it;
sprite_list_it_begin_fail:
	free(it);
	return NULL;
}

SpriteListIt* SpriteListItNext(SpriteListIt* _it) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_it) { return NULL; }

	// Advance to next valid element
	unordered_map_it_next(_it->_ptr);
	if (_it->_ptr) {
		// Update iterator contents
		size_t* idx = _it->_ptr->data;
		_it->data = free_list_get(_it->_list->_sprites, *idx);
		_it->id = (SpriteID)_it->_ptr->key;
		if (!_it->data) { 
			LUNA_DEBUG_WARN("Next iterator position yielded invalid sprite data!");
			LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			free(_it);
			return NULL;
		}
	}
	else {
		// Deallocate iterator
		free(_it);
		return NULL;
	}
	return _it;
}

SpriteListDepthIt* SpriteListDepthItBegin(SpriteList* _list) {
	LUNA_RETURN_CLEAR;
	// Error check
	SpriteListDepthIt* it = NULL;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto sprite_list_depth_it_begin_fail;
	}
	if (free_list_size(_list->_sprites) == 0) { 
		goto sprite_list_depth_it_begin_fail; 
	}

	// Create queue iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list iterator!");
		goto sprite_list_depth_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = priority_queue_it_begin(_list->_spriteDepthOrder);
	if (!it->_ptr) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list depth queue iterator!");
		goto sprite_list_depth_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DEBUG_WARN("Sprite list depth queue iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto sprite_list_depth_it_begin_fail; 
	}

	// Retrieve sprite data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_sprites, *idx);
	if (!it->data) { 
		LUNA_DEBUG_WARN("Sprite list iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto sprite_list_depth_it_begin_fail; 
	}
	it->id = (SpriteID)it->data->_id;

	return it;
sprite_list_depth_it_begin_fail:
	free(it);
	return NULL;
}

SpriteListDepthIt* SpriteListDepthItRBegin(SpriteList* _list) {
	LUNA_RETURN_CLEAR;
	// Error check
	SpriteListDepthIt* it = NULL;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto sprite_list_depth_rbegin_fail;
	}
	if (free_list_size(_list->_sprites) == 0) { 
		goto sprite_list_depth_rbegin_fail; 
	}

	// Create queue iterator
	it = calloc(1, sizeof *it);
	if (!it) { goto sprite_list_depth_rbegin_fail; }
	it->_list = _list;
	it->_ptr = priority_queue_it_rbegin(_list->_spriteDepthOrder);
	if (!it->_ptr) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate sprite list depth queue iterator!");
		goto sprite_list_depth_rbegin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DEBUG_WARN("Sprite list depth queue iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto sprite_list_depth_rbegin_fail; 
	}

	// Retrieve sprite data from list
	size_t* idx = it->_ptr->data;
	it->data = free_list_get(_list->_sprites, *idx);
	if (!it->data) { 
		LUNA_DEBUG_WARN("Sprite list iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto sprite_list_depth_rbegin_fail; 
	}
	it->id = (SpriteID)it->data->_id;

	return it;
sprite_list_depth_rbegin_fail:
	free(it);
	return NULL;
}

SpriteListDepthIt* SpriteListDepthItNext(SpriteListDepthIt* _it) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_it) { return NULL; }

	// Advance to next valid element
	priority_queue_it_next(_it->_ptr);
	if (_it->_ptr) {
		// Update iterator contents
		size_t* idx = _it->_ptr->data;
		_it->data = free_list_get(_it->_list->_sprites, *idx);
		if (!_it->data) { 
			LUNA_DEBUG_WARN("Next iterator position yielded invalid sprite data!");
			LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			free(_it);
			return NULL;
		}
		_it->id = (SpriteID)_it->data->_id;
	}
	else {
		// Deallocate iterator
		free(_it);
		return NULL;
	}
	return _it;
}

SpriteListDepthIt* SpriteListDepthItPrev(SpriteListDepthIt* _it) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_it) { return NULL; }

	// Advance to previous valid element
	priority_queue_it_prev(_it->_ptr);
	if (_it->_ptr) {
		// Update iterator contents
		size_t* idx = _it->_ptr->data;
		_it->data = free_list_get(_it->_list->_sprites, *idx);
		if (!_it->data) { 
			LUNA_DEBUG_WARN("Next iterator position yielded invalid sprite data!");
			LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			free(_it);
			return NULL;
		}
		_it->id = (SpriteID)_it->data->_id;
	}
	else {
		// Deallocate iterator
		free(_it);
		return NULL;
	}
	return _it;
}

SpriteID CreateSprite(SpriteList* _list, SpriteDesc _desc) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ID_NULL; 
	}

	SpriteID id = _luna_id_generate();
	Sprite spr = {
		._texture = _desc.texture,
		._position = _desc.position,
		._scale = _desc.scale,
		._origin = _desc.origin,
		._scrollSpeed = _desc.scrollSpeed,
		._scrollOffset = _desc.scrollOffset,
		._scrollTimer = { 0.f, 0.f },
		._tint = _desc.tint,
		._id = id,
		._depth = _desc.depth,
		._imageIndex = _desc.imageIndex,
		._imageNum = _desc.imageNum,
		._numRows = _desc.numRows,
		._numCols = _desc.numCols,
		._imageSpeed = _desc.imageSpeed,
		._rotation = _desc.rotation,
		._rotationSpeed = _desc.rotationSpeed,
		._timer = 0.f,
		._visible = _desc.visible
	};
	size_t idx = 0;
	if (!free_list_insert(_list->_sprites, &idx, &spr)) {
		LUNA_DEBUG_WARN("Failed to add sprite to list!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->_spriteIndices, id, &idx)) {
		LUNA_DEBUG_WARN("Failed to add sprite to index map!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		free_list_remove(_list->_sprites, idx);
		return ID_NULL;
	}
	if (_list->_depthSorting) {
		if (!priority_queue_push(_list->_spriteDepthOrder, spr._depth, &idx)) {
			LUNA_DEBUG_WARN("Failed to add sprite to depth queue!");
			LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
			free_list_remove(_list->_sprites, idx);
			unordered_map_delete(_list->_spriteIndices, id);
			return ID_NULL;
		}
	}
	return id;
}

void DestroySprite(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		if (_list->_depthSorting) {
			Sprite* spr = free_list_get(_list->_sprites, *idxPtr);
			if (spr) {
				priority_queue_remove(_list->_spriteDepthOrder, spr->_depth, idxPtr);
			}
		}
		free_list_remove(_list->_sprites, *idxPtr);
		unordered_map_delete(_list->_spriteIndices, _id);
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

void SetSpritePosition(SpriteList* _list, SpriteID _id, Vector2 _position) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_position = _position;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Vector2 GetSpritePosition(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	Vector2 ret = { 0.f, 0.f };
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ret; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		ret.x = sprite->_position.x;
		ret.y = sprite->_position.y;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return ret;
}

void SetSpriteDepth(SpriteList* _list, SpriteID _id, int _depth) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return;
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update depth & sort if necessary
		if (_list->_depthSorting) {
			priority_queue_remove(_list->_spriteDepthOrder, sprite->_depth, idxPtr);
			priority_queue_push(_list->_spriteDepthOrder, _depth, idxPtr);
		}
		sprite->_depth = _depth;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

int GetSpriteDepth(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_depth;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return 0;
}

void SetSpriteImageIndex(SpriteList* _list, SpriteID _id, int _imageIndex) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_imageIndex = _imageIndex;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

int GetSpriteImageIndex(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_imageIndex;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return 0;
}

void SetSpriteImageSpeed(SpriteList* _list, SpriteID _id, float _imageSpeed) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_imageSpeed = _imageSpeed;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

float GetSpriteImageSpeed(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0.f; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_imageSpeed;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return 0.f;
}

void SetSpriteVisible(SpriteList* _list, SpriteID _id, bool _visible) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_visible = _visible;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

bool GetSpriteVisible(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return false; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_visible;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return false;
}

void SetSpriteScrollSpeed(SpriteList* _list, SpriteID _id, Vector2 _scrollSpeed) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_scrollSpeed = _scrollSpeed;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Vector2 GetSpriteScrollSpeed(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return Vector2Zero(); 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_scrollSpeed;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return Vector2Zero();
}

void SetSpriteScrollOffset(SpriteList* _list, SpriteID _id, Vector2 _scrollOffset) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_scrollOffset = _scrollOffset;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Vector2 GetSpriteScrollOffset(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return Vector2Zero(); 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_scrollOffset;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return Vector2Zero();
}

void SetSpriteScale(SpriteList* _list, SpriteID _id, Vector2 _scale) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_scale = _scale;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Vector2 GetSpriteScale(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return Vector2Zero(); 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_scale;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return Vector2Zero();
}

void SetSpriteRotation(SpriteList* _list, SpriteID _id, Angle _rotation) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_rotation = _rotation;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Angle GetSpriteRotation(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ANGLE_MIN; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_rotation;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return ANGLE_MIN;
}

void SetSpriteRotationSpeed(SpriteList* _list, SpriteID _id, Angle _rotationSpeed) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_rotationSpeed = _rotationSpeed;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Angle GetSpriteRotationSpeed(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ANGLE_MIN; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_rotationSpeed;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return ANGLE_MIN;
}

void SetSpriteTint(SpriteList* _list, SpriteID _id, Color _tint) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Update property
		sprite->_tint = _tint;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

Color GetSpriteTint(SpriteList* _list, SpriteID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid sprite list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return BLACK; 
	}

	size_t* idxPtr = unordered_map_find(_list->_spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->_sprites, *idxPtr);

		// Get property
		return sprite->_tint;
	}
	else {
		LUNA_DEBUG_WARN("Invalid sprite id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
	return BLACK;
}