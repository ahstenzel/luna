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
	Texture2D tex_tile_default; GetTexture(LUNA_RESOURCES, "tiles/default.png", &tex_tile_default);
	TilemapDesc scene_first_tilemap_desc = {
		.texture = tex_tile_default,
		.texTileSize = {16, 16},
		.texTileSpacing = {0, 0},
		.texOffset = {0, 0},
		.sceneMapSize = {40, 30},
		.sceneOffset = {0, 0},
		.tint = WHITE,
		.depth = 0,
		.visible = true
	};
	TilemapList* scene_first_tilemap_list = GetSceneTilemapList(LUNA_SCENES, scene_first);
	TilemapID scene_first_tilemap = CreateTilemap(scene_first_tilemap_list, scene_first_tilemap_desc);
	TileIdx scene_first_tiles_map[1200];
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
	SetTilemapIndexAll(scene_first_tilemap_list, scene_first_tilemap, scene_first_tiles_map, 1200);

	// Create camera
	CameraList* scene_first_camera_list = GetSceneCameraList(LUNA_SCENES, scene_first);
	CameraDesc scene_first_camera_desc = {
		.offset = { GetScreenWidth() / 2.f, GetScreenHeight() / 2.f },
		.target = { GetScreenWidth() / 2.f, GetScreenHeight() / 2.f },
		.rotation = 0.f,
		.zoom = 1.f
	};
	CameraID scene_first_camera = CreateCamera(scene_first_camera_list, scene_first_camera_desc);
}

void scene_first_fn_push(SceneID _id) {
	Texture2D tex_spr_test; GetTexture(LUNA_RESOURCES, "sprites/face.png", &tex_spr_test);
	SpriteDesc dsc_spr_test_1 = {
		.texture = tex_spr_test,
		.position = { 64.0f, 64.0f },
		.scale = { 2.0f, 2.0f },
		.origin = { 0.0f, 0.0f },
		.scrollSpeed = { 8.f, 8.f },
		.scrollOffset = { 0.f, 0.f },
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
		.position = { 128.0f, 64.0f },
		.scale = { 2.0f, 2.0f },
		.origin = { 0.5f, 0.5f },
		.scrollSpeed = { 0.f, 0.f },
		.scrollOffset = { 0.f, 0.f },
		.tint = PINK,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 3,
		.numRows = 1,
		.numCols = 3,
		.imageSpeed = 5.0f,
		.rotation = 0.0f,
		.visible = true
	};
	SpriteDesc dsc_spr_test_3 = {
		.texture = tex_spr_test,
		.position = { 192.0f, 64.0f },
		.scale = { 2.0f, 2.0f },
		.origin = { 1.0f, 1.0f },
		.tint = RED,
		.depth = 0,
		.imageIndex = 0,
		.imageNum = 3,
		.numRows = 1,
		.numCols = 3,
		.imageSpeed = 15.0f,
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
	while (counter >= 4.f) {
		counter -= 4.f;
	}
	float v = sinf(2 * PI * counter / 4.f);
	Camera2D* camera = GetActiveCamera(GetSceneCameraList(LUNA_SCENES, _id));
	//camera->target.x = roundf((GetScreenWidth() / 2.f) + (8.f * v));

	/*
	unsigned char out[5] = "....\0";
	unsigned char dir = GetDirectionBitfield(LUNA_INPUTS, 0);
	if (dir & INPUT_MASK_DOWN)  out[0] = 'v';
	if (dir & INPUT_MASK_UP)    out[1] = '^';
	if (dir & INPUT_MASK_LEFT)  out[2] = '<';
	if (dir & INPUT_MASK_RIGHT) out[3] = '>';
	LUNA_DEBUG_LOG("[%s]", out);
	*/
}

void scene_first_fn_pop(SceneID _id) {
	LUNA_DEBUG_LOG("Pop!");
}