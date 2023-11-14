#include "luna/scene.h"

SceneList* CreateSceneList() {
	SceneList* list = calloc(1, sizeof *list);
	if (!list) { return NULL; }
	list->sceneStack = stack_create(SceneID);
	list->scenes = unordered_map_create(Scene);
	if (!list->scenes || !list->sceneStack) { 
		DestroySceneList(list);
		return NULL;
	}
	return list;
}

void UpdateSceneList(SceneList* _list, float _dt) {
	if (!_list) { return; }

	SceneID id = TopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene) {
			UpdateSpriteList(scene->spriteList, _dt);
			if (scene->updateFPtr) {
				scene->updateFPtr(id, _dt);
			}
		}
	}
}

void DrawSceneList(SceneList* _list) {
	if (!_list) { return; }

	SceneID id = TopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene) {
			DrawSpriteList(scene->spriteList);
		}
	}
}

void DestroySceneList(SceneList* _list) {
	if (_list) {
		stack_destroy(_list->sceneStack);
		unordered_map_destroy(_list->scenes);
		free(_list);
	}
}

SceneID CreateScene(SceneList* _list, SceneDesc _desc) {
	if (!_list) { return ID_NULL; }

	SceneID id = _luna_id_generate();
	Scene scene = {
		.spriteList = CreateSpriteList(_desc.depthSorting),
		.collisionMap = CreateCollisionMap(),
		.pushFPtr = _desc.pushFPtr,
		.topFPtr = _desc.topFPtr,
		.updateFPtr = _desc.updateFPtr,
		.popFPtr = _desc.popFPtr,
		.id = id
	};
	if (!scene.spriteList || !scene.collisionMap) {
		DestroySpriteList(scene.spriteList);
		DestroyCollisionMap(scene.collisionMap);
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
		DestroySpriteList(scene->spriteList);
		DestroyCollisionMap(scene->collisionMap);
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

	SceneID id = TopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene && scene->popFPtr) {
			scene->popFPtr(id);
		}
	}
	stack_pop(_list->sceneStack);

	id = TopScene(_list);
	if (id != ID_NULL) {
		Scene* scene = unordered_map_find(_list->scenes, id);
		if (scene && scene->topFPtr) {
			scene->topFPtr(id);
		}
	}
}

SceneID TopScene(SceneList* _list) {
	if (!_list) { return ID_NULL; }

	SceneID id = ID_NULL;
	while (1) {
		if (stack_size(_list->sceneStack) == 0) { break; }
		id = *(SceneID*)(stack_head(_list->sceneStack));
		if (unordered_map_find(_list->scenes, id)) { break; }
		stack_pop(_list->sceneStack);
	}
	
	return id;
}