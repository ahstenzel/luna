#include "luna/sprite.h"

void UpdateSprite(Sprite* _sprite, float _dt) {
	if (!_sprite) { return; }

	_sprite->timer = fmodf(_sprite->timer + _dt, _sprite->imageSpeed);
	if (_sprite->timer < _dt) {
		_sprite->imageIndex = (_sprite->imageIndex + 1) % _sprite->imageNum;
	}
}

void DrawSprite(Sprite* _sprite) {
	if (!_sprite) { return; }

	float width = _sprite->texture.width / (float)_sprite->numCols;
	float height = _sprite->texture.height / (float)_sprite->numRows;
	size_t col = _sprite->imageIndex % _sprite->numCols;
	size_t row = _sprite->imageIndex % _sprite->numRows;
	Rectangle rec = {
		width*col,
		height*row,
		width,
		height
	};
	DrawTextureRec(_sprite->texture, rec, _sprite->position, _sprite->tint);
}

SpriteList* CreateSpriteList(bool _depthSorting) {
	SpriteList* list = calloc(1, sizeof *list);
	if (!list) { return NULL; }
	list->depthSorting = _depthSorting;
	list->spriteIndices = unordered_map_create(size_t);
	list->sprites = free_list_create(Sprite);
	list->spriteDepthOrder = priority_queue_create(size_t);
	if (!list->spriteIndices || !list->sprites || !list->spriteDepthOrder) {
		DestroySpriteList(list);
		return NULL;
	}
	return list;
}

void DestroySpriteList(SpriteList* _list) {
	if (_list) {
		unordered_map_destroy(_list->spriteIndices);
		free_list_destroy(_list->sprites);
		priority_queue_destroy(_list->spriteDepthOrder);
		free(_list);
	}
}

void UpdateSpriteList(SpriteList* _list, float _dt) {
	if (!_list) { return; }

	for(free_list_it_t* it = free_list_it(_list->sprites); it; free_list_it_next(it)) {
		Sprite* sprite = it->data;
		UpdateSprite(sprite, _dt);
	}
}

void DrawSpriteList(SpriteList* _list) {
	if (!_list) { return; }

	if (_list->depthSorting) {
		for(priority_queue_it_t* it = priority_queue_it(_list->spriteDepthOrder); it; priority_queue_it_next(it)) {
			size_t idx = *(size_t*)(it->data);
			Sprite* sprite = free_list_get(_list->sprites, idx);
			if (sprite && sprite->visible) {
				DrawSprite(sprite);
			}
		}
	}
	else {
		for(free_list_it_t* it = free_list_it(_list->sprites); it; free_list_it_next(it)) {
			Sprite* sprite = it->data;
			if (sprite->visible) {
				DrawSprite(sprite);
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
		.tint = _desc.tint,
		.depth = _desc.depth,
		.imageIndex = _desc.imageIndex,
		.id = id,
		.imageNum = _desc.imageNum,
		.numRows = _desc.numRows,
		.numCols = _desc.numCols,
		.imageSpeed = _desc.imageSpeed,
		.timer = 0.f,
		.visible = _desc.visible
	};
	size_t* idxPtr = NULL;
	if (!free_list_insert(_list->sprites, idxPtr, &spr)) {
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->spriteIndices, id, *idxPtr)) {
		free_list_remove(_list->sprites, *idxPtr);
		return ID_NULL;
	}
	if (_list->depthSorting) {
		if (!priority_queue_push(_list->spriteDepthOrder, spr.depth, *idxPtr)) {
			free_list_remove(_list->sprites, *idxPtr);
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

void SetSpriteImageIndex(SpriteList* _list, SpriteID _id, int _imageIndex) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->imageIndex = _imageIndex;
	}
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

void SetSpriteVisible(SpriteList* _list, SpriteID _id, bool _visible) {
	if (!_list) { return; }

	size_t* idxPtr = unordered_map_find(_list->spriteIndices, _id);
	if (idxPtr) {
		Sprite* sprite = free_list_get(_list->sprites, *idxPtr);

		// Update property
		sprite->visible = _visible;
	}
}