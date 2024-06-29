#include "scenes.h"

SceneID scene_first;
SceneDesc scene_first_desc = {
	.depthSorting = true,
	.topFPtr = NULL,
	.updateFPtr = scene_first_fn_update,
	.popFPtr = scene_first_fn_pop,
	.pushFPtr = scene_first_fn_push
};

void GameRegisterScenes() {
	// ============================================================ First scene
	// Create scene
	scene_first = CreateScene(LUNA_SCENES, scene_first_desc);
	PushScene(LUNA_SCENES, scene_first);
	// Create tilemap
	LUNA_DBG_LOG("(GameRegisterScenes) Loading tilemap texture");
	Texture2D tex_tile_default; GetTexture(LUNA_RESOURCES, "tiles/default.png", &tex_tile_default);
	LUNA_DBG_LOG("(GameRegisterScenes) Create tilemap");
	TilemapDesc scene_first_tiles_desc = {
		.texture = tex_tile_default,
		.texTileSize = {16, 16},
		.texTileSpacing = {0, 0},
		.texOffset = {0, 0},
		.sceneMapSize = {40, 30},
		.sceneOffset = {0, 0},
		.tint = WHITE
	};
	Tilemap* scene_first_tiles = CreateSceneTilemap(LUNA_SCENES, scene_first, scene_first_tiles_desc);
	if (!scene_first_tiles) { LUNA_DBG_ERR("Failed to create tilemap!"); }
	unsigned int scene_first_tiles_map[1200];
	for(size_t i=0; i<1200; ++i) { 
		Vector2i pos = {(int32_t)i % 40, (int32_t)i / 40};
		if (pos.x == 0) {
			if (pos.y == 0) { scene_first_tiles_map[i] = 1; }
			else if (pos.y == 29) { scene_first_tiles_map[i] = 7; }
			else { scene_first_tiles_map[i] = 4; }
		}
		else if (pos.x == 39) {
			if (pos.y == 0) { scene_first_tiles_map[i] = 3; }
			else if (pos.y == 29) { scene_first_tiles_map[i] = 9; }
			else { scene_first_tiles_map[i] = 6; }
		}
		else {
			if (pos.y == 0) { scene_first_tiles_map[i] = 2; }
			else if (pos.y == 29) { scene_first_tiles_map[i] = 8; }
			else { scene_first_tiles_map[i] = 5; }
		} 
	}
	int ret = SetTilemapIndexAll(scene_first_tiles, scene_first_tiles_map, 1200);
	if (ret != 0) { LUNA_DBG_ERR("Failed to set tilemap data! Error (%d)", ret); }
}

void scene_first_fn_push(SceneID _id) {
	Texture2D tex_spr_test; GetTexture(LUNA_RESOURCES, "sprites/face.png", &tex_spr_test);
	SpriteDesc dsc_spr_test_1 = {
		.texture = tex_spr_test,
		.position = {64.0f, 64.0f},
		.scale = {2.0f, 2.0f},
		.origin = {0.0f, 0.0f},
		.tint = WHITE,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 3,
		.numRows = 1,
		.numCols = 3,
		.imageSpeed = 1.0f,
		.rotation = 0.0f,
		.visible = true
	};
	SpriteDesc dsc_spr_test_2 = {
		.texture = tex_spr_test,
		.position = {128.0f, 64.0f},
		.scale = {2.0f, 2.0f},
		.origin = {0.5f, 0.5f},
		.tint = PINK,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 3,
		.numRows = 1,
		.numCols = 3,
		.imageSpeed = 0.5f,
		.rotation = 10.0f,
		.visible = true
	};
	SpriteDesc dsc_spr_test_3 = {
		.texture = tex_spr_test,
		.position = {192.0f, 64.0f},
		.scale = {2.0f, 2.0f},
		.origin = {1.0f, 1.0f},
		.tint = RED,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 3,
		.numRows = 1,
		.numCols = 3,
		.imageSpeed = 0.25f,
		.rotation = 20.0f,
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