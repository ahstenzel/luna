#include "luna/std/debug.h"

#if defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)

#include <stdio.h>
#include <stdarg.h>

unsigned char _luna_g_dbg_log_mask = LUNA_DBG_LEVEL_ALL;

void _luna_dbg_log_write(const unsigned char level, char* fmt, ...) {
	if ((_luna_g_dbg_log_mask & level) != 0) {
		// Get variadic arguments
		va_list args;
		va_start(args, fmt);

		// Write to console
		char c = 'X';
		switch (level) {
			case (LUNA_DBG_LEVEL_STATUS):
				printf_s("[*] ");
				vprintf_s(fmt, args);
				printf_s("\n");
			break;
			case (LUNA_DBG_LEVEL_WARNING):
				c = '!';
			case (LUNA_DBG_LEVEL_ERROR):
				fprintf_s(stderr, "[%c] ", c);
				vfprintf_s(stderr, fmt, args);
				fprintf_s(stderr, "\n");
			break;
		}
	}
}

#endif