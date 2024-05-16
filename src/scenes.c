#include "scenes.h"

SceneID scene_first;
SceneDesc scene_first_desc = {
	.depthSorting = true,
	.topFPtr = scene_first_fn_top,
	.updateFPtr = scene_first_fn_update,
	.popFPtr = scene_first_fn_pop,
	.pushFPtr = NULL
};

void GameRegisterScenes() {
	scene_first = CreateScene(LUNA_SCENES, scene_first_desc);
	PushScene(LUNA_SCENES, scene_first);
}

void scene_first_fn_top(SceneID _id) {
	LUNA_DBG_LOG("Top!");
}

void scene_first_fn_update(SceneID _id, float _dt) {
	static float counter = 0.f;
	counter += _dt;
	if (counter >= 1.f) {
		//LUNA_DBG_LOG("Tick!");
		counter = 0.f;
	}

	unsigned char out[5] = "....\0";
	unsigned char dir = GetDirectionBitfield(LUNA_INPUTS, 0);
	if (dir & INPUT_MASK_DOWN)  out[0] = 'v';
	if (dir & INPUT_MASK_UP)    out[1] = '^';
	if (dir & INPUT_MASK_LEFT)  out[2] = '<';
	if (dir & INPUT_MASK_RIGHT) out[3] = '>';
	LUNA_DBG_LOG("[%s]", out);
}

void scene_first_fn_pop(SceneID _id) {
	LUNA_DBG_LOG("Pop!");
}