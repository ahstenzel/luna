#include "luna/sprite.h"

void _UpdateSprite(Sprite* _sprite, float _dt) {
	if (!_sprite) { return; }

	_sprite->_timer = fmodf(_sprite->_timer + _dt, _sprite->imageSpeed);
	if (_sprite->_timer < _dt) {
		_sprite->imageIndex = (_sprite->imageIndex + 1) % _sprite->imageNum;
	}
}

void _DrawSprite(Sprite* _sprite) {
	if (!_sprite) { return; }

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
	if (!list) { return NULL; }
	list->depthSorting = _depthSorting;
	list->spriteIndices = unordered_map_create(size_t);
	list->sprites = free_list_create(Sprite);
	list->spriteDepthOrder = priority_queue_create(size_t);
	if (!list->spriteIndices || !list->sprites || !list->spriteDepthOrder) {
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
	if (!_list) { return; }

	for(free_list_it_t* it = free_list_it(_list->sprites); it; free_list_it_next(it)) {
		Sprite* sprite = it->data;
		_UpdateSprite(sprite, _dt);
	}
}

void _DrawSpriteList(SpriteList* _list) {
	if (!_list) { return; }

	if (_list->depthSorting) {
		for(priority_queue_it_t* it = priority_queue_it(_list->spriteDepthOrder); it; priority_queue_it_next(it)) {
			size_t idx = *(size_t*)(it->data);
			Sprite* sprite = free_list_get(_list->sprites, idx);
			if (sprite && sprite->visible) {
				_DrawSprite(sprite);
			}
		}
	}
	else {
		for(free_list_it_t* it = free_list_it(_list->sprites); it; free_list_it_next(it)) {
			Sprite* sprite = it->data;
			if (sprite->visible) {
				_DrawSprite(sprite);
			}
		}
	}
}

SpriteID CreateSprite(SpriteList* _list, SpriteDesc _desc) {
	if (!_list) { return ID_NULL; }

	SpriteID id = _luna_id_generate();
	Sprite spr = {
		.texture = _desc.texture,
		.position = _desc.position,
		.scale = _desc.scale,
		.origin = _desc.origin,
		.tint = _desc.tint,
		.depth = _desc.depth,
		.imageIndex = _desc.imageIndex,
		.id = id,
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
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->spriteIndices, id, &idx)) {
		free_list_remove(_list->sprites, idx);
		return ID_NULL;
	}
	if (_list->depthSorting) {
		if (!priority_queue_push(_list->spriteDepthOrder, spr.depth, &idx)) {
			free_list_remove(_list->sprites, idx);
			unordered_map_delete(_list->spriteIndices, id);
			return ID_NULL;
		}
	}
	return id;
}

void DestroySprite(SpriteList* _list, SpriteID _id) {
	if (!_list) { return; }

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
}

void SetSpritePosition(SpriteList* _list, SpriteID _id, Vector2 _position) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->position = _position;
	}
}

Vector2 GetSpritePosition(SpriteList* _list, SpriteID _id) {
	Vector2 ret = { 0.f, 0.f };
	if (!_list) { return ret; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		ret.x = sprite->position.x;
		ret.y = sprite->position.y;
	}
	return ret;
}

void SetSpriteDepth(SpriteList* _list, SpriteID _id, int _depth) {
	if (!_list) { return; }

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
}

int GetSpriteDepth(SpriteList* _list, SpriteID _id) {
	if (!_list) { return 0; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->depth;
	}
	return 0;
}

void SetSpriteImageIndex(SpriteList* _list, SpriteID _id, int _imageIndex) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->imageIndex = _imageIndex;
	}
}

int GetSpriteImageIndex(SpriteList* _list, SpriteID _id) {
	if (!_list) { return 0; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->imageIndex;
	}
	return 0;
}

void SetSpriteImageSpeed(SpriteList* _list, SpriteID _id, float _imageSpeed) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->imageSpeed = _imageSpeed;
	}
}

float GetSpriteImageSpeed(SpriteList* _list, SpriteID _id) {
	if (!_list) { return 0.f; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->imageSpeed;
	}
	return 0.f;
}

void SetSpriteVisible(SpriteList* _list, SpriteID _id, bool _visible) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->visible = _visible;
	}
}

bool GetSpriteVisible(SpriteList* _list, SpriteID _id) {
	if (!_list) { return false; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Get property
		return sprite->visible;
	}
	return false;
}