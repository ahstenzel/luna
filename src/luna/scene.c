#include "luna/scene.h"

SceneList* _CreateSceneList() {
	SceneList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate scene list!");
		return NULL; 
	}

	list->_sceneStack = stack_create(SceneID);
	list->_scenes = unordered_map_create(Scene);
	if (!list->_scenes || !list->_sceneStack) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate scene list containers!");
		_DestroySceneList(list);
		return NULL;
	}
	return list;
}

void _UpdateSceneList(SceneList* _list, float _dt) {
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	SceneID id = GetTopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->_scenes, id);
		if (scene) {
			_UpdateSpriteList(scene->_spriteList, _dt);
			if (scene->_updateFPtr) {
				scene->_updateFPtr(id, _dt);
			}
		}
	}
}

void _DrawSceneList(SceneList* _list) {
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	SceneID id = GetTopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->_scenes, id);
		if (scene) {
			Camera2D* camera = GetActiveCamera(scene->_cameraList);
			if (camera) { BeginMode2D(*camera); }
			_DrawTilemapList(scene->_tilemapList);
			_DrawSpriteList(scene->_spriteList);
			if (camera) { EndMode2D(); }
		}
	}
}

void _DestroySceneList(SceneList* _list) {
	if (_list) {
		// Clear all scenes in the stack
		while(stack_size(_list->_sceneStack) > 0) {
			SceneID id = *(SceneID*)(stack_head(_list->_sceneStack));
			if (id != ID_NULL) {
				Scene* scene = unordered_map_find(_list->_scenes, id);
				if (scene && scene->_popFPtr) {
					scene->_popFPtr(id);
				}
				DestroyScene(_list, id);
			}
			stack_pop(_list->_sceneStack);
		}

		// Clear memory
		stack_destroy(_list->_sceneStack);
		unordered_map_destroy(_list->_scenes);
		free(_list);
	}
}

size_t GetSceneListSize(SceneList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0; 
	}
	return unordered_map_size(_list->_scenes);
}

size_t GetSceneStackSize(SceneList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return 0; 
	}
	return stack_size(_list->_sceneStack);
}

SceneListIt* SceneListItBegin(SceneList* _list) {
	LUNA_RETURN_CLEAR;
	// Error check
	SceneListIt* it = NULL;
	if (!_list) {
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		goto scene_list_it_begin_fail;
	}
	if (unordered_map_size(_list->_scenes) == 0) { 
		goto scene_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to scene sprite list iterator!");
		goto scene_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->_scenes);
	if (!it->_ptr) { 
		LUNA_ABORT(LUNA_ERROR_STATUS_BAD_ALLOC, "Failed to allocate scene list index map iterator!");
		goto scene_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DEBUG_WARN("Scene list index map iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto scene_list_it_begin_fail; 
	}

	// Retrieve scene data from list
	it->data = (Scene*)it->_ptr->data;
	it->id = (SceneID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DEBUG_WARN("Scene list iterator points to invalid data!");
		LUNA_RETURN_SET(LUNA_RETURN_CONTAINER_FAILURE);
		goto scene_list_it_begin_fail; 
	}

	return it;
scene_list_it_begin_fail:
	free(it);
	return NULL;
}

SceneListIt* SceneListItNext(SceneListIt* _it) {
	LUNA_RETURN_CLEAR;
	// Error check
	if (!_it) { return NULL; }

	// Advance to next valid element
	unordered_map_it_next(_it->_ptr);
	if (_it->_ptr) {
		// Update iterator contents
		_it->data = (Scene*)_it->_ptr->data;
		_it->id = (SceneID)_it->_ptr->key;
		if (!_it->data) { 
			LUNA_DEBUG_WARN("Next iterator position yielded invalid scene data!");
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

SceneID CreateScene(SceneList* _list, SceneDesc _desc) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ID_NULL; 
	}

	SceneID id = _luna_id_generate();
	Scene scene = {
		._spriteList = _CreateSpriteList(_desc.depthSorting),
		._collisionList = _CreateCollisionList(),
		._tilemapList = _CreateTilemapList(_desc.depthSorting),
		._cameraList = _CreateCameraList(),
		._pushFPtr = _desc.pushFPtr,
		._topFPtr = _desc.topFPtr,
		._updateFPtr = _desc.updateFPtr,
		._popFPtr = _desc.popFPtr,
		._id = id
	};
	if (!scene._spriteList || !scene._collisionList || !scene._tilemapList) {
		_DestroySpriteList(scene._spriteList);
		_DestroyCollisionList(scene._collisionList);
		_DestroyTilemapList(scene._tilemapList);
		_DestroyCameraList(scene._cameraList);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->_scenes, id, &scene)) {
		return ID_NULL;
	}
	return id;
}

void DestroyScene(SceneList* _list, SceneID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (scene) {
		_DestroySpriteList(scene->_spriteList);
		_DestroyCollisionList(scene->_collisionList);
		_DestroyTilemapList(scene->_tilemapList);
		_DestroyCameraList(scene->_cameraList);
		unordered_map_delete(_list->_scenes, _id);
	}
}

void PushScene(SceneList* _list, SceneID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (scene) {
		stack_push(_list->_sceneStack, &_id);
		if (scene->_pushFPtr) {
			scene->_pushFPtr(_id);
		}
		if (scene->_topFPtr) {
			scene->_topFPtr(_id);
		}
	}
	else {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
	}
}

void PopScene(SceneList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return; 
	}

	SceneID id = GetTopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->_scenes, id);
		if (scene && scene->_popFPtr) {
			scene->_popFPtr(id);
		}
	}
	stack_pop(_list->_sceneStack);

	id = GetTopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->_scenes, id);
		if (scene && scene->_topFPtr) {
			scene->_topFPtr(id);
		}
	}
}

SceneID GetTopScene(SceneList* _list) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return ID_NULL; 
	}

	SceneID id = ID_NULL;
	while (1) {
		if (stack_size(_list->_sceneStack) == 0) { break; }
		id = *(SceneID*)(stack_head(_list->_sceneStack));
		if (unordered_map_find(_list->_scenes, id)) { break; }
		LUNA_DEBUG_WARN("Scene id (%d) does not exist! Popping the stack & using the next value...", (int)id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		stack_pop(_list->_sceneStack);
	}
	
	return id;
}

SpriteList* GetSceneSpriteList(SceneList* _list, SceneID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return NULL; 
	}
	if (_id == ID_NULL) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}
	return scene->_spriteList;
}

CollisionList* GetSceneCollisionList(SceneList* _list, SceneID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return NULL; 
	}
	if (_id == ID_NULL) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}
	return scene->_collisionList;
}

TilemapList* GetSceneTilemapList(SceneList* _list, SceneID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return NULL;
	}
	if (_id == ID_NULL) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}
	return scene->_tilemapList;
}

CameraList* GetSceneCameraList(SceneList* _list, SceneID _id) {
	LUNA_RETURN_CLEAR;
	if (!_list) { 
		LUNA_DEBUG_WARN("Invalid scene list reference!");
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_REFERENCE);
		return NULL; 
	}
	if (_id == ID_NULL) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DEBUG_WARN("Invalid scene id (%d)!", (int)_id);
		LUNA_RETURN_SET(LUNA_RETURN_INVALID_ID);
		return NULL; 
	}
	return scene->_cameraList;
}