#include "scenes.h"

SceneID scene_first;
SceneDesc scene_first_desc = {
	.depthSorting = false,
	.topFPtr = NULL,
	.updateFPtr = scene_first_fn_update,
	.popFPtr = scene_first_fn_pop,
	.pushFPtr = scene_first_fn_push
};

void GameRegisterScenes() {
	scene_first = CreateScene(LUNA_SCENES, scene_first_desc);
	PushScene(LUNA_SCENES, scene_first);
}

void scene_first_fn_push(SceneID _id) {
	Texture2D tex_spr_test; GetTexture(LUNA_RESOURCES, "block.png", &tex_spr_test);
	SpriteDesc dsc_spr_test_1 = {
		.texture = tex_spr_test,
		.position = {64.0f, 64.0f},
		.tint = WHITE,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 2,
		.numRows = 1,
		.numCols = 2,
		.imageSpeed = 1.0f,
		.visible = true
	};
	SpriteDesc dsc_spr_test_2 = {
		.texture = tex_spr_test,
		.position = {128.0f, 64.0f},
		.tint = PINK,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 2,
		.numRows = 1,
		.numCols = 2,
		.imageSpeed = 0.5f,
		.visible = true
	};
	SpriteDesc dsc_spr_test_3 = {
		.texture = tex_spr_test,
		.position = {192.0f, 64.0f},
		.tint = RED,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 2,
		.numRows = 1,
		.numCols = 2,
		.imageSpeed = 0.25f,
		.visible = true
	};
	SpriteID test_sprite_1 = CreateSprite(LUNA_SPRITES, dsc_spr_test_1);
	SpriteID test_sprite_2 = CreateSprite(LUNA_SPRITES, dsc_spr_test_2);
	SpriteID test_sprite_3 = CreateSprite(LUNA_SPRITES, dsc_spr_test_3);
}

void scene_first_fn_update(SceneID _id, float _dt) {
	static float counter = 0.f;
	counter += _dt;
	if (counter >= 1.f) {
		//LUNA_DBG_LOG("Tick!");
		counter = 0.f;
	}

	/*
	unsigned char out[5] = "....\0";
	unsigned char dir = GetDirectionBitfield(LUNA_INPUTS, 0);
	if (dir & INPUT_MASK_DOWN)  out[0] = 'v';
	if (dir & INPUT_MASK_UP)    out[1] = '^';
	if (dir & INPUT_MASK_LEFT)  out[2] = '<';
	if (dir & INPUT_MASK_RIGHT) out[3] = '>';
	LUNA_DBG_LOG("[%s]", out);
	*/
}

void scene_first_fn_pop(SceneID _id) {
	LUNA_DBG_LOG("Pop!");
}