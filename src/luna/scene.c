#include "luna/scene.h"

SceneList* _CreateSceneList() {
	SceneList* list = calloc(1, sizeof *list);
	if (!list) { 
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate scene list!");
		return NULL; 
	}

	list->sceneStack = stack_create(SceneID);
	list->scenes = unordered_map_create(Scene);
	if (!list->scenes || !list->sceneStack) { 
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
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene) {
			_UpdateSpriteList(scene->spriteList, _dt);
			if (scene->updateFPtr) {
				scene->updateFPtr(id, _dt);
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
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene) {
			Camera2D* camera = GetActiveCamera(scene->cameraList);
			if (camera) { BeginMode2D(*camera); }
			_DrawTilemapList(scene->tilemapList);
			_DrawSpriteList(scene->spriteList);
			if (camera) { EndMode2D(); }
		}
	}
}

void _DestroySceneList(SceneList* _list) {
	if (_list) {
		// Clear all scenes in the stack
		while(stack_size(_list->sceneStack) > 0) {
			SceneID id = *(SceneID*)(stack_head(_list->sceneStack));
			if (id != ID_NULL) {
				Scene* scene = unordered_map_find(_list->scenes, id);
				if (scene && scene->popFPtr) {
					scene->popFPtr(id);
				}
				DestroyScene(_list, id);
			}
			stack_pop(_list->sceneStack);
		}

		// Clear memory
		stack_destroy(_list->sceneStack);
		unordered_map_destroy(_list->scenes);
		free(_list);
	}
}

SceneID CreateScene(SceneList* _list, SceneDesc _desc) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return ID_NULL; 
	}

	SceneID id = _luna_id_generate();
	Scene scene = {
		.spriteList = _CreateSpriteList(_desc.depthSorting),
		.collisionList = _CreateCollisionList(),
		.tilemapList = _CreateTilemapList(_desc.depthSorting),
		.cameraList = _CreateCameraList(),
		.pushFPtr = _desc.pushFPtr,
		.topFPtr = _desc.topFPtr,
		.updateFPtr = _desc.updateFPtr,
		.popFPtr = _desc.popFPtr,
		.id = id
	};
	if (!scene.spriteList || !scene.collisionList || !scene.tilemapList) {
		_DestroySpriteList(scene.spriteList);
		_DestroyCollisionList(scene.collisionList);
		_DestroyTilemapList(scene.tilemapList);
		_DestroyCameraList(scene.cameraList);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->scenes, id, &scene)) {
		return ID_NULL;
	}
	return id;
}

void DestroyScene(SceneList* _list, SceneID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return; 
	}

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (scene) {
		_DestroySpriteList(scene->spriteList);
		_DestroyCollisionList(scene->collisionList);
		_DestroyTilemapList(scene->tilemapList);
		_DestroyCameraList(scene->cameraList);
		unordered_map_delete(_list->scenes, _id);
	}
}

void PushScene(SceneList* _list, SceneID _id) {
	if (!_list) { 
		LUNA_DBG_WARN("Invalid scene list reference!");
		return; 
	}

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (scene) {
		stack_push(_list->sceneStack, &_id);
		if (scene->pushFPtr) {
			scene->pushFPtr(_id);
		}
		if (scene->topFPtr) {
			scene->topFPtr(_id);
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
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene && scene->popFPtr) {
			scene->popFPtr(id);
		}
	}
	stack_pop(_list->sceneStack);

	id = GetTopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene && scene->topFPtr) {
			scene->topFPtr(id);
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
		if (stack_size(_list->sceneStack) == 0) { break; }
		id = *(SceneID*)(stack_head(_list->sceneStack));
		if (unordered_map_find(_list->scenes, id)) { break; }
		LUNA_DBG_WARN("Scene id (%d) does not exist! Popping the stack & using the next value...", (int)id);
		stack_pop(_list->sceneStack);
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

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->spriteList;
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

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->collisionList;
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

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->tilemapList;
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

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (!scene) {
		LUNA_DBG_WARN("Invalid scene id (%d)!", (int)_id);
		return NULL; 
	}
	return scene->cameraList;
}