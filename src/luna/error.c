#include "luna/error.h"
#include "luna/platform.h"
#include <stdlib.h>

#if defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)
	#include <stdio.h>
	#if defined(LUNA_CMP_MSVC)
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
		#undef WIN32_LEAN_AND_MEAN
	#elif defined(LUNA_CMP_GCC) || defined(LUNA_CMP_CLANG)
		#include <execinfo.h>
	#endif
#endif

unsigned char _luna_g_err_mask = LUNA_ERR_STATUS_NONE;

void _luna_print_stack() {
#if defined(LUNA_DEBUG) && !defined(LUNA_SUPPRESS_LOG)
	fprintf_s(stderr, "(Stack trace here TODO)\n");
#endif
}

void _luna_err_abort(unsigned int status) {
	if ((_luna_g_err_mask & status) != 0) {
		_luna_print_stack();
		exit(status);
	}
}