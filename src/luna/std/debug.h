#ifndef LUNA_STD_DEBUG_H
#define LUNA_STD_DEBUG_H
/**
 * debug.h
 * Debug logging & tracing definitions.
*/

// Debug marker
#if !defined(NDEBUG)
#define LUNA_DEBUG
#endif

#if defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)

// Debug level flags
#define LUNA_DBG_LEVEL_STATUS 0x01  // Low-level status message.
#define LUNA_DBG_LEVEL_WARNING 0x02 // Mid-level warning message - ignoring may lead to issues.
#define LUNA_DBG_LEVEL_ERROR 0x04   // High-level error message - catastrophic failure.
#define LUNA_DBG_LEVEL_ALL 0xFF     // Mask for all message levels.

// Global debug variables
extern unsigned char _luna_g_dbg_log_mask;  // Mask for disabling certain debug messages.

/// @brief Write a string to the debug output.
/// @param level Message severity level
/// @param fmt Format string
/// @param ... Format arguments
void _luna_dbg_log_write(const unsigned char level, char* fmt, ...);

/// @brief Write a debug info message.
#define LUNA_DBG_LOG(s, ...) _luna_dbg_log_write(LUNA_DBG_LEVEL_STATUS, s, ##__VA_ARGS__)

/// @brief Write a debug warning message.
#define LUNA_DBG_WARN(s, ...) _luna_dbg_log_write(LUNA_DBG_LEVEL_WARNING, s, ##__VA_ARGS__)

/// @brief Write a debug error message.
#define LUNA_DBG_ERR(s, ...) _luna_dbg_log_write(LUNA_DBG_LEVEL_ERROR, s, ##__VA_ARGS__)

/// @brief Set the bitmask that defines what debug messages are shown.
#define LUNA_DBG_SET_SEVERITY(x) _luna_g_dbg_log_mask = x

#else

#define LUNA_DBG_LOG(s, ...) ((void)0)
#define LUNA_DBG_WARN(s, ...) ((void)0)
#define LUNA_DBG_ERR(s, ...) ((void)0)
#define LUNA_DBG_SET_SEVERITY(x) ((void)0)

#endif // defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)

#endif