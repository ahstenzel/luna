#ifndef LUNA_ERROR__H
#define LUNA_ERROR__H
/**
 * error.h
 * Error handling functionality.
*/
#include "luna/std/debug.h"

//----------------------------------------------------------------------------------
// Return codes
//----------------------------------------------------------------------------------

// Code definitions
#define LUNA_RETURN_OK 					0		// Return code for successful execution.
#define LUNA_RETURN_CRITICAL_ERROR 		1		// Return code for an unhandled critical error.
#define LUNA_RETURN_INVALID_REFERENCE 	2		// Return code for an invalid pointer.
#define LUNA_RETURN_INVALID_ID			3		// Return code for an invalid unique ID.
#define LUNA_RETURN_INVALID_PARAMETER	4		// Return code for an improperly formatted function parameter.
#define LUNA_RETURN_CONTAINER_FAILURE 	5		// Return code for an internal failure in data containers.
#define LUNA_RETURN_RESOURCE_FAILURE	6		// Return code for an internal failure in the resource management system.

// Global variables
extern unsigned int _luna_g_return_code;	// Last recorded function return code.

/// @brief Get the last recorded function return code.
#define LUNA_RETURN _luna_g_return_code

/// @brief Clear the last recorded function return code.
#define LUNA_RETURN_CLEAR _luna_g_return_code = LUNA_RETURN_OK

/// @brief Record the current function return code.
#define LUNA_RETURN_SET(x) _luna_g_return_code = x


//----------------------------------------------------------------------------------
// Error raising
//----------------------------------------------------------------------------------

// Code definitions
#define LUNA_ERROR_STATUS_NONE 		0x00	// Mask for no errors.
#define LUNA_ERROR_STATUS_ALL 		0xFF	// Mask for all errors.
#define LUNA_ERROR_STATUS_GENERIC 	0x01	// Mask for uncategorized general error.
#define LUNA_ERROR_STATUS_BAD_ALLOC 0x02	// Mask for failed memory allocation.
#define LUNA_ERROR_STATUS_FILESYS 	0x04	// Mask for filesystem error.

// Global variables
extern unsigned char _luna_g_error_status_mask;  	// Mask for disabling certain error types

/// @brief Raise the given error status and abort the program if they are not masked off.
/// @param status Status code
void _luna_error_abort(unsigned int status);

/// @brief Prints the call stack to stderr.
void _luna_print_stack();

/// @brief Set the bitmask that defines which error codes should exit the program
#define LUNA_ERROR_SET_SEVERITY(x) _luna_g_error_status_mask = x

/// @brief Print an error message, then either exit the program (if the error type hasn't been masked), or set
///	@brief the global return code to indicate a critical unhandled error and continue execution.
/// @param x Error status code
/// @param s Format string
/// @param ... Format arguments
#define LUNA_ABORT(x, s, ...) { \
	LUNA_DEBUG_ERROR(s, ##__VA_ARGS__); \
	_luna_error_abort(x); \
	LUNA_RETURN_SET(LUNA_RETURN_CRITICAL_ERROR); \
}

#endif