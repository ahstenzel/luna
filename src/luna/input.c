#include "luna/input.h"

InputSettingsList* CreateInputSettingsList(InputSlotDesc* _desc, size_t _num) {
	InputSettingsList* list = calloc(1, sizeof *list);
	if (!list || !_desc || _num > MAX_INPUT_SLOTS) { 
#ifdef LUNA_DEBUG
		if (!list)  { LUNA_DBG_ERR("(CreateInputSettingsList): Failed to allocate list!"); }
		if (!_desc) { LUNA_DBG_ERR("(CreateInputSettingsList): Null descriptor!"); }
		if (_num > MAX_INPUT_SLOTS) { LUNA_DBG_ERR("(CreateInputSettingsList): Too many input descriptors!"); }
#endif
		return NULL; 
	}
	list->num = _num;
	for(size_t slot=0; slot<_num; ++slot) {
		list->inputs[slot].gamepadDeadzone = _desc[slot].gamepadDeadzone;
		list->inputs[slot].gamepadIndex = _desc[slot].gamepadIndex;
		list->inputs[slot].mode = _desc[slot].mode;
	}
	return list;
}

void DestroyInputSettingsList(InputSettingsList* _list) {
	if (_list) {
		free(_list);
	}
}

void UpdateInputSettingsList(InputSettingsList* _list) {
	// Error check
	if (!_list) { return; }

	for(int slot=0; slot<_list->num; ++slot) {
		switch(_list->inputs[slot].mode) {
			case INPUT_MODE_PENDING: 
				// Check for keyboard input
				if (!_IsKeyboardTaken(_list)) {
					if (GetKeyPressed() != KEY_NULL) {
						_list->inputs[slot].mode = INPUT_MODE_KEYBOARD;
						break;
					}
				}

				// Check for gamepad input
				for(int gamepad=0; gamepad<MAX_GAMEPADS; ++gamepad) {
					if (!_IsGamepadTaken(_list, gamepad) && _GetGamepadPressed(_list, slot, gamepad, NULL) != -1) {
						_list->inputs[slot].mode = INPUT_MODE_GAMEPAD;
						_list->inputs[slot].gamepadIndex = gamepad;
						for(int axis=0; axis<MAX_GAMEPAD_AXIS; ++axis) {
							_list->inputs[slot].gamepadAxisLatch[axis] = 2.0f;
						}
						_SetGamepadAxisLatches(_list, slot);
					}
				}
			break;
			case INPUT_MODE_GAMEPAD: 
				// Update axis latches
				_SetGamepadAxisLatches(_list, slot);
			break;
			default: break;
		}
	}
}

Vector2 GetDirection(InputSettingsList* _list, int _slot, bool _aligned) {
	// Error check
	if (!_list || _slot >= _list->num) { return Vector2Zero(); }

	Vector2 input = Vector2Zero();
	if (_list->inputs[_slot].mode == INPUT_MODE_GAMEPAD) {
		int gamepad = _list->inputs[_slot].gamepadIndex;
		if (GetGamepadAxisCount(gamepad) > 0) {
			Vector2 value = Vector2Zero();
			value.x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
			value.y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
			if (Vector2Length(value) >= _list->inputs[_slot].gamepadDeadzone) {
				if (_aligned) {
					input.x = (fabsf(value.x) > GAMEPAD_AXIS_SECTOR) ? (signbit(value.x) ? -1.f : 1.f) : 0.f;
					input.y = (fabsf(value.y) > GAMEPAD_AXIS_SECTOR) ? (signbit(value.y) ? -1.f : 1.f) : 0.f;
				}
				else {
					input = value;
				}
			}
		}
		else {
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_UP]))    { input.y -= 1.f; }
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_LEFT]))  { input.x -= 1.f; }
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_DOWN]))  { input.y += 1.f; }
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_RIGHT])) { input.x += 1.f; }
		}
	}
	else {
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_UP]))    { input.y -= 1.f; }
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_LEFT]))  { input.x -= 1.f; }
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_DOWN]))  { input.y += 1.f; }
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_RIGHT])) { input.x += 1.f; }
	}
	return Vector2Normalize(input);
}

unsigned char GetDirectionBitfield(InputSettingsList* _list, int _slot) {
	// Error check
	if (!_list || _slot >= _list->num) { return 0; }

	unsigned char input = 0;
	if (_list->inputs[_slot].mode == INPUT_MODE_GAMEPAD) {
		int gamepad = _list->inputs[_slot].gamepadIndex;
		if (GetGamepadAxisCount(gamepad) > 0) {
			Vector2 value = Vector2Zero();
			value.x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
			value.y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
			if (Vector2Length(value) >= _list->inputs[_slot].gamepadDeadzone) {
				if (value.y < -GAMEPAD_AXIS_SECTOR) { input |= INPUT_MASK_UP; }
				if (value.x < -GAMEPAD_AXIS_SECTOR) { input |= INPUT_MASK_LEFT; }
				if (value.y > GAMEPAD_AXIS_SECTOR)  { input |= INPUT_MASK_DOWN; }
				if (value.x > GAMEPAD_AXIS_SECTOR)  { input |= INPUT_MASK_RIGHT; }
			}
		}
		else {
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_UP]))    { input |= INPUT_MASK_UP; }
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_LEFT]))  { input |= INPUT_MASK_LEFT; }
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_DOWN]))  { input |= INPUT_MASK_DOWN; }
			if (IsGamepadButtonDown(gamepad, _list->inputs[_slot].gamepadMapping[INPUT_RIGHT])) { input |= INPUT_MASK_RIGHT; }
		}
	}
	else {
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_UP]))    { input |= INPUT_MASK_UP; }
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_LEFT]))  { input |= INPUT_MASK_LEFT; }
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_DOWN]))  { input |= INPUT_MASK_DOWN; }
		if (IsKeyDown(_list->inputs[_slot].keyboardMapping[INPUT_RIGHT])) { input |= INPUT_MASK_RIGHT; }
	}
	return input;
}

bool GetInputPressed(InputSettingsList* _list, int _slot, Input _input) {
	// Error check
	if (!_list || _slot >= _list->num) { return 0; }

	if (_list->inputs[_slot].mode == INPUT_MODE_GAMEPAD) {
		int mappedInput = _list->inputs[_slot].gamepadMapping[_input];
		return IsGamepadButtonPressed(_list->inputs[_slot].gamepadIndex, mappedInput);
	}
	else {
		int mappedInput = _list->inputs[_slot].keyboardMapping[_input];
		return IsKeyPressed(mappedInput);
	}
}

bool GetInputDown(InputSettingsList* _list, int _slot, Input _input) {
	// Error check
	if (!_list || _slot >= _list->num) { return 0; }

	if (_list->inputs[_slot].mode == INPUT_MODE_GAMEPAD) {
		int mappedInput = _list->inputs[_slot].gamepadMapping[_input];
		return IsGamepadButtonDown(_list->inputs[_slot].gamepadIndex, mappedInput);
	}
	else {
		int mappedInput = _list->inputs[_slot].keyboardMapping[_input];
		return IsKeyDown(mappedInput);
	}
}

bool GetInputReleased(InputSettingsList* _list, int _slot, Input _input) {
	// Error check
	if (!_list || _slot >= _list->num) { return 0; }

	if (_list->inputs[_slot].mode == INPUT_MODE_GAMEPAD) {
		int mappedInput = _list->inputs[_slot].gamepadMapping[_input];
		return IsGamepadButtonReleased(_list->inputs[_slot].gamepadIndex, mappedInput);
	}
	else {
		int mappedInput = _list->inputs[_slot].keyboardMapping[_input];
		return IsKeyReleased(mappedInput);
	}
}

int _GetGamepadPressed(InputSettingsList* _list, int _slot, int _gamepad, bool* _axis) {
	if (IsGamepadAvailable(_gamepad)) {
		for(int button=1; button<=MAX_GAMEPAD_BUTTONS; ++button ) {
			if (IsGamepadButtonPressed(_gamepad, button)) { 
				if (_axis) { *_axis = false; }
				return button; 
			}
		}
		for(int axis=0; axis<=GetGamepadAxisCount(_gamepad); ++axis) {
			float val = GetGamepadAxisMovement(_gamepad, axis);
			if (fabsf(val - _list->inputs[_slot].gamepadAxisLatch[axis]) > _list->inputs[_slot].gamepadDeadzone) {
				if (_axis) { *_axis = true; }
				return axis;
			}
		}
	}
	if (_axis) { *_axis = false; }
	return -1;
}

int GetKeyboardMapping(InputSettingsList* _list, int _slot, Input _input) {
	// Error check
	if (!_list || _slot >= _list->num) { return -1; }
	return _list->inputs[_slot].keyboardMapping[_input];
}

void SetKeyboardMapping(InputSettingsList* _list, int _slot, Input _input, int _mapping) {
	// Error check
	if (!_list || _slot >= _list->num) { return; }
	_list->inputs[_slot].keyboardMapping[_input] = _mapping;
}

int GetGamepadMapping(InputSettingsList* _list, int _slot, Input _input) {
	// Error check
	if (!_list || _slot >= _list->num) { return -1; }
	return _list->inputs[_slot].gamepadMapping[_input];
}

void SetGamepadMapping(InputSettingsList* _list, int _slot, Input _input, int _mapping) {
	// Error check
	if (!_list || _slot >= _list->num) { return; }
	_list->inputs[_slot].gamepadMapping[_input] = _mapping;
}

float GetGamepadDeadzone(InputSettingsList* _list, int _slot) {
	// Error check
	if (!_list || _slot >= _list->num) { return 0.f; }
	return _list->inputs[_slot].gamepadDeadzone;
}

void SetGamepadDeadzone(InputSettingsList* _list, int _slot, float _deadzone) {
	// Error check
	if (!_list || _slot >= _list->num) { return; }
	_list->inputs[_slot].gamepadDeadzone = _deadzone;
}

InputMode GetInputMode(InputSettingsList* _list, int _slot) {
	// Error check
	if (!_list || _slot >= _list->num) { return INPUT_MODE_NONE; }
	return _list->inputs[_slot].mode;
}

void SetInputMode(InputSettingsList* _list, int _slot, InputMode _mode) {
	// Error check
	if (!_list || _slot >= _list->num) { return; }
	_list->inputs[_slot].mode = _mode;
}

void SetGamepadIndex(InputSettingsList* _list, int _slot, int _gamepad) {
	// Error check
	if (!_list || _slot >= _list->num) { return; }
	_list->inputs[_slot].gamepadIndex = _gamepad;
}

bool _IsGamepadTaken(InputSettingsList* _list, int _gamepad) {
	for(int slot=0; slot<_list->num; ++slot) {
		if (_list->inputs[slot].gamepadIndex == _gamepad) {
			return true;
		}
	}
	return false;
}

bool _IsKeyboardTaken(InputSettingsList* _list) {
	for(int slot=0; slot<_list->num; ++slot) {
		if (_list->inputs[slot].mode == INPUT_MODE_KEYBOARD) {
			return true;
		}
	}
	return false;
}

bool _ScanForGamepads(InputSettingsList* _list, int _slot) {
	for(int gamepad=0; gamepad<MAX_GAMEPADS; ++gamepad) {
		if (IsGamepadAvailable(gamepad) && !_IsGamepadTaken(_list, gamepad)) {
			_list->inputs[_slot].gamepadIndex = gamepad;
			return true;
		}
	}
	_list->inputs[_slot].gamepadIndex = -1;
	return false;
}

void _SetGamepadAxisLatches(InputSettingsList* _list, int _slot) {
	int gamepad = _list->inputs[_slot].gamepadIndex;
	if (IsGamepadAvailable(gamepad) && _list->inputs[_slot].mode == INPUT_MODE_GAMEPAD) {
		for(int axis=0; axis<GetGamepadAxisCount(gamepad); ++axis) {
			float val = GetGamepadAxisMovement(gamepad, axis);
			if (fabsf(val) - _list->inputs[_slot].gamepadAxisLatch[axis] > 0.05f) {
				_list->inputs[_slot].gamepadAxisLatch[axis] = val;
			}
		}
	}
}