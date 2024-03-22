#include "luna/game.h"

int main() {
	GameDesc gameDesc = {
		.width = 640,
		.height = 480,
		.name = "LUNA"
	};
	ResourceListDesc resourceDesc = {
		.resourceFile = "",
		.resourcePassword = ""
	};
	InputDesc inputDesc = {
		.gamepadDeadzone = 0.1f,
		.gamepadIndex = -1,
		.mode = INPUT_MODE_KEYBOARD,
		.keyboardMapping[INPUT_A] = KEY_Z,
		.keyboardMapping[INPUT_B] = KEY_X,
		.keyboardMapping[INPUT_X] = KEY_A,
		.keyboardMapping[INPUT_Y] = KEY_S,
		.keyboardMapping[INPUT_L] = KEY_LEFT_SHIFT,
		.keyboardMapping[INPUT_R] = KEY_RIGHT_SHIFT,
		.keyboardMapping[INPUT_START] = KEY_ENTER,
		.keyboardMapping[INPUT_SELECT] = KEY_TAB,
		.keyboardMapping[INPUT_UP] = KEY_UP,
		.keyboardMapping[INPUT_DOWN] = KEY_DOWN,
		.keyboardMapping[INPUT_LEFT] = KEY_LEFT,
		.keyboardMapping[INPUT_RIGHT] = KEY_RIGHT
	};

	CreateGame(gameDesc, resourceDesc, &inputDesc, 1);
	UpdateGame();
	DestroyGame();
	return 0;
}