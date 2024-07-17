#include "luna/error.h"
#include "luna/platform.h"
#include <stdlib.h>

#if defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)
	#include <stdio.h>
	#if defined(LUNA_OS_WINDOWS)
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
		#undef WIN32_LEAN_AND_MEAN
	#else
		#include <execinfo.h>
	#endif
#endif

unsigned int _luna_g_return_code = LUNA_RETURN_OK;

unsigned char _luna_g_error_status_mask = LUNA_ERROR_STATUS_NONE;

void _luna_print_stack() {
#if defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)
	fprintf_s(stderr, "(Stack trace here TODO)\n");
#endif
}

void _luna_error_abort(unsigned int status) {
	if ((_luna_g_error_status_mask & status) != 0) {
		_luna_print_stack();
		exit(status);
	}
}