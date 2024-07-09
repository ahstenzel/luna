#ifndef LUNA_ERROR_H
#define LUNA_ERROR_H
/**
 * error.h
 * Error handling functionality.
*/
#include "luna/std/debug.h"

// Status codes
#define LUNA_ERR_STATUS_NONE 0x00		// Mask for no errors.
#define LUNA_ERR_STATUS_ALL 0xFF		// Mask for all errors.
#define LUNA_ERR_STATUS_GENERIC 0x01	// Mask for uncategorized general error.
#define LUNA_ERR_STATUS_BAD_ALLOC 0x02	// Mask for failed memory allocation.
#define LUNA_ERR_STATUS_FILESYS 0x04	// Mask for filesystem error.

// Global variables
extern unsigned char _luna_g_err_mask;  // Mask for disabling certain error types

/// @brief Raise the given error status and abort the program if they are not masked off.
/// @param status Status code
void _luna_err_abort(unsigned int status);

/// @brief Prints the call stack to stderr.
void _luna_print_stack();

/// @brief Set the bitmask that defines which error codes should exit the program
#define LUNA_ERR_SET_EXIT(x) _luna_g_err_mask = x

/// @brief Print an error message then exit the program.
/// @param x Error status code
/// @param s Format string
/// @param ... Format arguments
#define LUNA_RAISE_ERR(x, s, ...) { \
	LUNA_DBG_ERR(s, ##__VA_ARGS__); \
	_luna_err_abort(x); \
}

#endif