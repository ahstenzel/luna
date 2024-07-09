#ifndef LUNA_INPUT_H
#define LUNA_INPUT_H
/**
 * input.h
 * Abstraction layer for reading controller inputs.
*/
#include "luna/common.h"

// Defines copied from internal raylib definitions
#define MAX_INPUT_SLOTS      4  // Maximum number of input slots
#define MAX_GAMEPADS         4  // Maximum number of gamepads supported
#define MAX_GAMEPAD_AXIS     8  // Maximum number of axis supported (per gamepad)
#define MAX_GAMEPAD_BUTTONS 32  // Maximum number of buttons supported (per gamepad)
#define INPUT_MASK_UP    0b00000001
#define INPUT_MASK_LEFT  0b00000010
#define INPUT_MASK_DOWN  0b00000100
#define INPUT_MASK_RIGHT 0b00001000
#define GAMEPAD_AXIS_SECTOR 0.573576f

typedef enum {
	INPUT_A = 0,
	INPUT_B,
	INPUT_X,
	INPUT_Y,
	INPUT_L,
	INPUT_R,
	INPUT_START,
	INPUT_SELECT,
	INPUT_UP,
	INPUT_DOWN,
	INPUT_LEFT,
	INPUT_RIGHT
} Input;

#define INPUT_NUMBER 12

typedef enum {
	INPUT_MODE_NONE = 0,
	INPUT_MODE_PENDING,
	INPUT_MODE_KEYBOARD,
	INPUT_MODE_GAMEPAD
} InputMode;

typedef struct {
	int keyboardMapping[INPUT_NUMBER];
	int gamepadMapping[INPUT_NUMBER];
	float gamepadAxisLatch[MAX_GAMEPAD_AXIS];
	float gamepadDeadzone;
	int gamepadIndex;
	InputMode mode;
} InputSlot;

typedef struct {
	int keyboardMapping[INPUT_NUMBER];
	int gamepadMapping[INPUT_NUMBER];
	float gamepadDeadzone;
	int gamepadIndex;
	InputMode mode;
} InputSlotDesc;

typedef struct {
	InputSlot inputs[MAX_INPUT_SLOTS];
	size_t num;
} InputSettingsList;

/// @brief Create a new input list.
/// @param _desc Array of input slot descriptors
/// @param _num Number of elements in the array
/// @return Input list pointer
InputSettingsList* _CreateInputSettingsList(InputSlotDesc* _desc, size_t _num);

/// @brief Deallocate an input list.
/// @param _list Input list pointer
void _DestroyInputSettingsList(InputSettingsList* _list);

/// @brief Update the state of all input slots in the list.
/// @param _list Input list pointer
void _UpdateInputSettingsList(InputSettingsList* _list);

/// @brief Get the movement direction for the given input slot as a vector.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _aligned If using a gamepad axis, snap the vector to 8-directional movement
/// @return Movement vector
Vector2 GetDirection(InputSettingsList* _list, unsigned int _slot, bool _aligned);

/// @brief Get the movement direction for the given input slot as a bitfield of actively pressed inputs.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @return Bitfield
unsigned char GetDirectionBitfield(InputSettingsList* _list, unsigned int _slot);

/// @brief Check if the given input is pressed for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @return True if pressed on this frame
bool GetInputPressed(InputSettingsList* _list, unsigned int _slot, Input _input);

/// @brief Check if the given input is held for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @return True if held on this frame
bool GetInputDown(InputSettingsList* _list, unsigned int _slot, Input _input);

/// @brief Check if the given input is released for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @return True if released on this frame
bool GetInputReleased(InputSettingsList* _list, unsigned int _slot, Input _input);

/// @brief Get the keyboard mapping for the input flag in the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @return Keyboard scancode
int GetKeyboardMapping(InputSettingsList* _list, unsigned int _slot, Input _input);

/// @brief Set the keyboard mapping for the input flag in the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @param _mapping Keyboard scancode
void SetKeyboardMapping(InputSettingsList* _list, unsigned int _slot, Input _input, int _mapping);

/// @brief Get the gamepad mapping for the input flag in the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @return Gamepad scancode
int GetGamepadMapping(InputSettingsList* _list, unsigned int _slot, Input _input);

/// @brief Set the gamepad mapping for the input flag in the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _input Input enum
/// @param _mapping Gamepad scancode
void SetGamepadMapping(InputSettingsList* _list, unsigned int _slot, Input _input, int _mapping);

/// @brief Get the axis deadzone for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @return Gamepad deadzone
float GetGamepadDeadzone(InputSettingsList* _list, unsigned int _slot);

/// @brief Set the axis deadzone for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _deadzone Gamepad deadzone
void SetGamepadDeadzone(InputSettingsList* _list, unsigned int _slot, float _deadzone);

/// @brief Get the input mode for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @return Input mode enum
InputMode GetInputMode(InputSettingsList* _list, unsigned int _slot);

/// @brief Set the input mode for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _mode Input mode enum
void SetInputMode(InputSettingsList* _list, unsigned int _slot, InputMode _mode);

/// @brief Set the gamepad index for the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _gamepad Gamepad index
void SetGamepadIndex(InputSettingsList* _list, unsigned int _slot, int _gamepad);

/// @brief Get the most recently altered control state for the given gamepad.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @param _gamepad Gamepad index
/// @param _axis Destination for if an axis has changed, rather than a button
/// @return Gamepad scancode, or -1 for no change
int _GetGamepadPressed(InputSettingsList* _list, unsigned int _slot, int _gamepad, bool* _axis);

/// @brief Check if the gamepad index is assigned to any input slot.
/// @param _list Input list pointer
/// @param _gamepad Gamepad index
/// @return True if any input slot has the gamepad index assigned to it
bool _IsGamepadTaken(InputSettingsList* _list, int _gamepad);

/// @brief Check if the keyboard is assigned to any input slot.
/// @param _list Input list pointer
/// @return True if any input slot is in Keyboard mode
bool _IsKeyboardTaken(InputSettingsList* _list);

/// @brief Scan for the first available & unclaimed gamepad to assign to the given input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
/// @return True if a gamepad was found & assigned
bool _ScanForGamepads(InputSettingsList* _list, unsigned int _slot);

/// @brief Record the last known value for each axis of the gamepad in the input slot.
/// @param _list Input list pointer
/// @param _slot Input slot
void _SetGamepadAxisLatches(InputSettingsList* _list, unsigned int _slot);

#endif