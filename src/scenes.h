#ifndef GAME_SCENES_H
#define GAME_SCENES_H
/**
 * scenes.h
 * Define all scenes in the game.
*/
#include "luna/game.h"

void GameRegisterScenes();

extern SceneID scene_first;
extern SceneDesc scene_first_desc;
void scene_first_fn_push(SceneID _id);
void scene_first_fn_update(SceneID _id, float _dt);
void scene_first_fn_pop(SceneID _id);

#endif