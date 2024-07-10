#include "luna/scene.h"

SceneList* _CreateSceneList() {
	SceneList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate scene list!");
		return NULL; 
	}

	list->_sceneStack = stack_create(SceneID);
	list->_scenes = unordered_map_create(Scene);
	if (!list->_scenes || !list->_sceneStack) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate scene list containers!");
		_DestroySceneList(list);
		return NULL;
	}
	return list;
}

void _UpdateSceneList(SceneList* _list, float _dt) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
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
		LUNA_DBG_WARN("Invalid scene list reference!");
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
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return 0; 
	}
	return unordered_map_size(_list->_scenes);
}

size_t GetSceneStackSize(SceneList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return 0; 
	}
	return stack_size(_list->_sceneStack);
}

SceneListIt* SceneListItBegin(SceneList* _list) {
	// Error check
	SceneListIt* it = NULL;
	if (!_list) {
		LUNA_DBG_WARN("Invalid scene list reference!");
		goto scene_list_it_begin_fail;
	}
	if (unordered_map_size(_list->_scenes) == 0) { 
		goto scene_list_it_begin_fail; 
	}

	// Create map iterator
	it = calloc(1, sizeof *it);
	if (!it) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to scene sprite list iterator!");
		goto scene_list_it_begin_fail; 
	}
	it->_list = _list;
	it->_ptr = unordered_map_it(_list->_scenes);
	if (!it->_ptr) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate scene list index map iterator!");
		goto scene_list_it_begin_fail; 
	}
	if (!it->_ptr->data) { 
		LUNA_DBG_WARN("Scene list index map iterator points to invalid data!");
		goto scene_list_it_begin_fail; 
	}

	// Retrieve scene data from list
	it->data = (Scene*)it->_ptr->data;
	it->id = (SceneID)it->_ptr->key;
	if (!it->data) { 
		LUNA_DBG_WARN("Scene list iterator points to invalid data!");
		goto scene_list_it_begin_fail; 
	}

	return it;
scene_list_it_begin_fail:
	free(it);
	return NULL;
}

void SceneListItNext(SceneListIt** _it) {
	// Error check
	if (!_it || !(*_it)) { return; }

	// Advance to next valid element
	SceneListIt* it = *_it;
	unordered_map_it_next(it->_ptr);
	if (it->_ptr) {
		// Update iterator contents
		it->data = (Scene*)it->_ptr->data;
		it->id = (SceneID)it->_ptr->key;
		if (!it->data) { 
			LUNA_DBG_WARN("Next iterator position yielded invalid scene data!");
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

SceneID CreateScene(SceneList* _list, SceneDesc _desc) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
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
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
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
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
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
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
	}
}

void PopScene(SceneList* _list) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
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
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return ID_NULL; 
	}

	SceneID id = ID_NULL;
	while (1) {
		if (stack_size(_list->_sceneStack) == 0) { break; }
		id = *(SceneID*)(stack_head(_list->_sceneStack));
		if (unordered_map_find(_list->_scenes, id)) { break; }
		LUNA_DBG_WARN("Scene id (%d) does not exist! Popping the stack & using the next value...", (int)id);
		stack_pop(_list->_sceneStack);
	}
	
	return id;
}

SpriteList* GetSceneSpriteList(SceneList* _list, SceneID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return NULL; 
	}
	if (_id == ID_NULL) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->_spriteList;
}

CollisionList* GetSceneCollisionList(SceneList* _list, SceneID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return NULL; 
	}
	if (_id == ID_NULL) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->_collisionList;
}

TilemapList* GetSceneTilemapList(SceneList* _list, SceneID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return NULL;
	}
	if (_id == ID_NULL) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->_tilemapList;
}

CameraList* GetSceneCameraList(SceneList* _list, SceneID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return NULL; 
	}
	if (_id == ID_NULL) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}

	Scene* scene = unordered_map_find(_list->_scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->_cameraList;
}