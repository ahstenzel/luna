#include "luna/input.h"

InputSettingsList* _CreateInputSettingsList(InputSlotDesc* _desc, size_t _num) {
	// Error check
	if (!_desc) {
		LUNA_DBG_WARN("Invalid descriptor array!");
		return NULL;
	}
	if (_num > MAX_INPUT_SLOTS) {
		LUNA_DBG_WARN("Too many input descriptors!");
		return NULL;
	}

	// Create input list
	InputSettingsList* list = calloc(1, sizeof *list);
	if (!list) {
		LUNA_RAISE_ERR(LUNA_ERR_STATUS_BAD_ALLOC, "Failed to allocate input list!");
		return NULL;
	}
	list->_num = _num;
	for(size_t slot=0; slot<_num; ++slot) {
		list->_inputs[slot]._gamepadDeadzone = _desc[slot]._gamepadDeadzone;
		list->_inputs[slot]._gamepadIndex = _desc[slot]._gamepadIndex;
		list->_inputs[slot]._mode = _desc[slot]._mode;
		memcpy_s(&(list->_inputs[slot]._keyboardMapping), INPUT_NUMBER*sizeof(int), &(_desc[slot]._keyboardMapping), INPUT_NUMBER*sizeof(int));
		memcpy_s(&(list->_inputs[slot]._gamepadMapping), INPUT_NUMBER*sizeof(int), &(_desc[slot]._gamepadMapping), INPUT_NUMBER*sizeof(int));
	}
	return list;
}

void _DestroyInputSettingsList(InputSettingsList* _list) {
	if (_list) {
		free(_list);
	}
}

void _UpdateInputSettingsList(InputSettingsList* _list) {
	// Error check
	if (!_list) { 
		LUNA_DBG_WARN("Invalid input list reference!");
		return; 
	}

	for(int slot=0; slot<_list->_num; ++slot) {
		switch(_list->_inputs[slot]._mode) {
			case INPUT_MODE_PENDING: 
				// Check for keyboard input
				if (!_IsKeyboardTaken(_list)) {
					if (GetKeyPressed() != KEY_NULL) {
						_list->_inputs[slot]._mode = INPUT_MODE_KEYBOARD;
						break;
					}
				}

				// Check for gamepad input
				for(int gamepad=0; gamepad<MAX_GAMEPADS; ++gamepad) {
					if (!_IsGamepadTaken(_list, gamepad) && _GetGamepadPressed(_list, slot, gamepad, NULL) != -1) {
						_list->_inputs[slot]._mode = INPUT_MODE_GAMEPAD;
						_list->_inputs[slot]._gamepadIndex = gamepad;
						for(int axis=0; axis<MAX_GAMEPAD_AXIS; ++axis) {
							_list->_inputs[slot]._gamepadAxisLatch[axis] = 2.0f;
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

Vector2 GetDirection(InputSettingsList* _list, unsigned int _slot, bool _aligned) {
	// Error check
	Vector2 input = Vector2Zero();
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return input;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return input;
	}

	if (_list->_inputs[_slot]._mode == INPUT_MODE_GAMEPAD) {
		int gamepad = _list->_inputs[_slot]._gamepadIndex;
		if (GetGamepadAxisCount(gamepad) > 0) {
			Vector2 value = Vector2Zero();
			value.x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
			value.y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
			if (Vector2Length(value) >= _list->_inputs[_slot]._gamepadDeadzone) {
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
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_UP]))    { input.y -= 1.f; }
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_LEFT]))  { input.x -= 1.f; }
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_DOWN]))  { input.y += 1.f; }
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_RIGHT])) { input.x += 1.f; }
		}
	}
	else {
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_UP]))    { input.y -= 1.f; }
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_LEFT]))  { input.x -= 1.f; }
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_DOWN]))  { input.y += 1.f; }
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_RIGHT])) { input.x += 1.f; }
	}
	return Vector2Normalize(input);
}

unsigned char GetDirectionBitfield(InputSettingsList* _list, unsigned int _slot) {
	// Error check
	unsigned char input = 0;
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return input;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return input;
	}

	if (_list->_inputs[_slot]._mode == INPUT_MODE_GAMEPAD) {
		int gamepad = _list->_inputs[_slot]._gamepadIndex;
		if (GetGamepadAxisCount(gamepad) > 0) {
			Vector2 value = Vector2Zero();
			value.x = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_X);
			value.y = GetGamepadAxisMovement(gamepad, GAMEPAD_AXIS_LEFT_Y);
			if (Vector2Length(value) >= _list->_inputs[_slot]._gamepadDeadzone) {
				if (value.y < -GAMEPAD_AXIS_SECTOR) { input |= INPUT_MASK_UP; }
				if (value.x < -GAMEPAD_AXIS_SECTOR) { input |= INPUT_MASK_LEFT; }
				if (value.y > GAMEPAD_AXIS_SECTOR)  { input |= INPUT_MASK_DOWN; }
				if (value.x > GAMEPAD_AXIS_SECTOR)  { input |= INPUT_MASK_RIGHT; }
			}
		}
		else {
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_UP]))    { input |= INPUT_MASK_UP; }
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_LEFT]))  { input |= INPUT_MASK_LEFT; }
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_DOWN]))  { input |= INPUT_MASK_DOWN; }
			if (IsGamepadButtonDown(gamepad, _list->_inputs[_slot]._gamepadMapping[INPUT_RIGHT])) { input |= INPUT_MASK_RIGHT; }
		}
	}
	else {
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_UP]))    { input |= INPUT_MASK_UP; }
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_LEFT]))  { input |= INPUT_MASK_LEFT; }
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_DOWN]))  { input |= INPUT_MASK_DOWN; }
		if (IsKeyDown(_list->_inputs[_slot]._keyboardMapping[INPUT_RIGHT])) { input |= INPUT_MASK_RIGHT; }
	}
	return input;
}

bool GetInputPressed(InputSettingsList* _list, unsigned int _slot, Input _input) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return false;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return false;
	}

	if (_list->_inputs[_slot]._mode == INPUT_MODE_GAMEPAD) {
		int mappedInput = _list->_inputs[_slot]._gamepadMapping[_input];
		return IsGamepadButtonPressed(_list->_inputs[_slot]._gamepadIndex, mappedInput);
	}
	else {
		int mappedInput = _list->_inputs[_slot]._keyboardMapping[_input];
		return IsKeyPressed(mappedInput);
	}
}

bool GetInputDown(InputSettingsList* _list, unsigned int _slot, Input _input) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return false;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return false;
	}

	if (_list->_inputs[_slot]._mode == INPUT_MODE_GAMEPAD) {
		int mappedInput = _list->_inputs[_slot]._gamepadMapping[_input];
		return IsGamepadButtonDown(_list->_inputs[_slot]._gamepadIndex, mappedInput);
	}
	else {
		int mappedInput = _list->_inputs[_slot]._keyboardMapping[_input];
		return IsKeyDown(mappedInput);
	}
}

bool GetInputReleased(InputSettingsList* _list, unsigned int _slot, Input _input) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return false;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return false;
	}

	if (_list->_inputs[_slot]._mode == INPUT_MODE_GAMEPAD) {
		int mappedInput = _list->_inputs[_slot]._gamepadMapping[_input];
		return IsGamepadButtonReleased(_list->_inputs[_slot]._gamepadIndex, mappedInput);
	}
	else {
		int mappedInput = _list->_inputs[_slot]._keyboardMapping[_input];
		return IsKeyReleased(mappedInput);
	}
}

int _GetGamepadPressed(InputSettingsList* _list, unsigned int _slot, int _gamepad, bool* _axis) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		goto get_gamepad_pressed_fail;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		goto get_gamepad_pressed_fail;
	}

	if (IsGamepadAvailable(_gamepad)) {
		for(int button=1; button<=MAX_GAMEPAD_BUTTONS; ++button ) {
			if (IsGamepadButtonPressed(_gamepad, button)) { 
				if (_axis) { *_axis = false; }
				return button; 
			}
		}
		for(int axis=0; axis<=GetGamepadAxisCount(_gamepad); ++axis) {
			float val = GetGamepadAxisMovement(_gamepad, axis);
			if (fabsf(val - _list->_inputs[_slot]._gamepadAxisLatch[axis]) > _list->_inputs[_slot]._gamepadDeadzone) {
				if (_axis) { *_axis = true; }
				return axis;
			}
		}
	}

get_gamepad_pressed_fail:
	if (_axis) { *_axis = false; }
	return -1;
}

int GetKeyboardMapping(InputSettingsList* _list, unsigned int _slot, Input _input) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return -1;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return -1;
	}
	return _list->_inputs[_slot]._keyboardMapping[_input];
}

void SetKeyboardMapping(InputSettingsList* _list, unsigned int _slot, Input _input, int _mapping) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return;
	}
	_list->_inputs[_slot]._keyboardMapping[_input] = _mapping;
}

int GetGamepadMapping(InputSettingsList* _list, unsigned int _slot, Input _input) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return -1;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return -1;
	}
	return _list->_inputs[_slot]._gamepadMapping[_input];
}

void SetGamepadMapping(InputSettingsList* _list, unsigned int _slot, Input _input, int _mapping) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return;
	}
	_list->_inputs[_slot]._gamepadMapping[_input] = _mapping;
}

float GetGamepadDeadzone(InputSettingsList* _list, unsigned int _slot) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return 0.f;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return 0.f;
	}
	return _list->_inputs[_slot]._gamepadDeadzone;
}

void SetGamepadDeadzone(InputSettingsList* _list, unsigned int _slot, float _deadzone) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return;
	}
	_list->_inputs[_slot]._gamepadDeadzone = _deadzone;
}

InputMode GetInputMode(InputSettingsList* _list, unsigned int _slot) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return INPUT_MODE_NONE;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return INPUT_MODE_NONE;
	}
	return _list->_inputs[_slot]._mode;
}

void SetInputMode(InputSettingsList* _list, unsigned int _slot, InputMode _mode) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return;
	}
	_list->_inputs[_slot]._mode = _mode;
}

void SetGamepadIndex(InputSettingsList* _list, unsigned int _slot, int _gamepad) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return;
	}
	_list->_inputs[_slot]._gamepadIndex = _gamepad;
}

bool _IsGamepadTaken(InputSettingsList* _list, int _gamepad) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return false;
	}

	for(int slot=0; slot<_list->_num; ++slot) {
		if (_list->_inputs[slot]._gamepadIndex == _gamepad) {
			return true;
		}
	}
	return false;
}

bool _IsKeyboardTaken(InputSettingsList* _list) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return false;
	}

	for(int slot=0; slot<_list->_num; ++slot) {
		if (_list->_inputs[slot]._mode == INPUT_MODE_KEYBOARD) {
			return true;
		}
	}
	return false;
}

bool _ScanForGamepads(InputSettingsList* _list, unsigned int _slot) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return false;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return false;
	}

	for(int gamepad=0; gamepad<MAX_GAMEPADS; ++gamepad) {
		if (IsGamepadAvailable(gamepad) && !_IsGamepadTaken(_list, gamepad)) {
			_list->_inputs[_slot]._gamepadIndex = gamepad;
			return true;
		}
	}
	_list->_inputs[_slot]._gamepadIndex = -1;
	return false;
}

void _SetGamepadAxisLatches(InputSettingsList* _list, unsigned int _slot) {
	// Error check
	if (!_list) {
		LUNA_DBG_WARN("Invalid input list reference!");
		return;
	}
	if (_slot >= _list->_num) {
		LUNA_DBG_WARN("Invalid input slot (%d)!", _slot);
		return;
	}
	
	int gamepad = _list->_inputs[_slot]._gamepadIndex;
	if (IsGamepadAvailable(gamepad) && _list->_inputs[_slot]._mode == INPUT_MODE_GAMEPAD) {
		for(int axis=0; axis<GetGamepadAxisCount(gamepad); ++axis) {
			float val = GetGamepadAxisMovement(gamepad, axis);
			if (fabsf(val) - _list->_inputs[_slot]._gamepadAxisLatch[axis] > 0.05f) {
				_list->_inputs[_slot]._gamepadAxisLatch[axis] = val;
			}
		}
	}
}