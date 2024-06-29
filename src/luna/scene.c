#include "luna/scene.h"

SceneList* _CreateSceneList() {
	SceneList* list = calloc(1, sizeof *list);
	if (!list) { return NULL; }
	list->sceneStack = stack_create(SceneID);
	list->scenes = unordered_map_create(Scene);
	if (!list->scenes || !list->sceneStack) { 
		_DestroySceneList(list);
		return NULL;
	}
	return list;
}

void _UpdateSceneList(SceneList* _list, float _dt) {
	if (!_list) { return; }

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
	if (!_list) { return; }

	SceneID id = GetTopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene) {
			_DrawTilemap(scene->tilemap);
			_DrawSpriteList(scene->spriteList);
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
	if (!_list) { return ID_NULL; }

	SceneID id = _luna_id_generate();
	Scene scene = {
		.spriteList = _CreateSpriteList(_desc.depthSorting),
		.collisionMap = _CreateCollisionMap(),
		.tilemap = NULL,
		.pushFPtr = _desc.pushFPtr,
		.topFPtr = _desc.topFPtr,
		.updateFPtr = _desc.updateFPtr,
		.popFPtr = _desc.popFPtr,
		.id = id
	};
	if (!scene.spriteList || !scene.collisionMap) {
		_DestroySpriteList(scene.spriteList);
		_DestroyCollisionMap(scene.collisionMap);
		return ID_NULL;
	}
	if (!unordered_map_insert(_list->scenes, id, &scene)) {
		return ID_NULL;
	}
	return id;
}

void DestroyScene(SceneList* _list, SceneID _id) {
	if (!_list) { return; }

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (scene) {
		_DestroySpriteList(scene->spriteList);
		_DestroyCollisionMap(scene->collisionMap);
		_DestroyTilemap(scene->tilemap);
		unordered_map_delete(_list->scenes, _id);
	}
}

void PushScene(SceneList* _list, SceneID _id) {
	if (!_list) { return; }

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
}

void PopScene(SceneList* _list) {
	if (!_list) { return; }

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
	if (!_list) { return ID_NULL; }

	SceneID id = ID_NULL;
	while (1) {
		if (stack_size(_list->sceneStack) == 0) { break; }
		id = *(SceneID*)(stack_head(_list->sceneStack));
		if (unordered_map_find(_list->scenes, id)) { break; }
		LUNA_DBG_WARN("(GetTopScene) Scene id (%d) does not exist! Popping the stack & using the next value...", (int)id);
		stack_pop(_list->sceneStack);
	}
	
	return id;
}

Tilemap* CreateSceneTilemap(SceneList* _list, SceneID _id, TilemapDesc _desc) {
	if (!_list) { return NULL; }

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (scene) {
		// Create new tilemap
		Tilemap* tmp = _CreateTilemap(_desc);
		if (!tmp) {
			LUNA_DBG_ERR("(CreateSceneTilemap) Failed to create tilemap!");
			return NULL;
		}

		// Overwrite existing tilemap
		if (scene->tilemap) {
			_DestroyTilemap(scene->tilemap);
			free(scene->tilemap);
		}
		scene->tilemap = tmp;
		return scene->tilemap;
	}
	return NULL;
}

Tilemap* GetSceneTilemap(SceneList* _list, SceneID _id) {
	if (!_list) { return NULL; }

	Scene* scene = unordered_map_find(_list->scenes, _id);
	if (scene) { return scene->tilemap; }
	return NULL;
}

Tilemap* _GetTopSceneTilemap(SceneList* _list) {
	if (!_list) { return NULL; }
	SceneID id = GetTopScene(_list);
	if (id == ID_NULL) { return NULL; }
	Scene* scene = unordered_map_find(_list->scenes, id);
	if (!scene) { return NULL; }
	return scene->tilemap;
}

SpriteList* _GetTopSceneSprites(SceneList* _list) {
	if (!_list) { return NULL; }
	SceneID id = GetTopScene(_list);
	if (id == ID_NULL) { return NULL; }
	Scene* scene = unordered_map_find(_list->scenes, id);
	if (!scene) { return NULL; }
	return scene->spriteList;
}

CollisionList* _GetTopSceneCollisions(SceneList* _list) {
	if (!_list) { return NULL; }
	SceneID id = GetTopScene(_list);
	if (id == ID_NULL) { return NULL; }
	Scene* scene = unordered_map_find(_list->scenes, id);
	if (!scene) { return NULL; }
	return scene->collisionMap;
}