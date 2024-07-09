#ifndef LUNA_PLATFORM_H
#define LUNA_PLATFORM_H
/**
 * platform.h
 * Platform-specific preprocessor definitions.
*/

// Detect compiler
#if defined(__clang__)
#define LUNA_CMP_CLANG
#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined(__INTEL_COMPILER)
#define LUNA_CMP_GCC
#elif defined(_MSC_VER)
#define LUNA_CMP_MSVC
#elif
#define LUNA_CMP_UNKNOWN
#endif

// Detect architecture
#if defined(__arm__) || defined(__thumb__) ||\
	defined(_M_ARM)  || defined(_M_ARMT)
#define LUNA_ARCH_ARM
#elif defined(__aarch64__) || defined (_M_ARM64)
#define LUNA_ARCH_ARM64
#elif defined(i386) || defined(__i386) || defined(__i386__) ||\
	defined(i486) || defined(__i486) || defined(__i486__) ||\
	defined(i586) || defined(__i586) || defined(__i586__) ||\
	defined(i686) || defined(__i686) || defined(__i686__) ||\
	defined(_M_IX86) || defined(_M_X86) || defined(_X86_)
#define LUNA_ARCH_X86
#elif defined(__x86_64) || defined(__x86_64__) ||\
	defined(__amd64) || defined(__amd64__) ||\
	defined(_M_X64) || defined(_M_AMD64)
#define LUNA_ARCH_X64
#else
#define LUNA_ARCH_UNKNOWN
#endif

// Detect OS
#if defined(_WIN64) || defined(_WIN32)
#define LUNA_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
#define LUNA_OS_MAC
#elif defined(__linux__)
#define LUNA_OS_LINUX
#else
#define LUNA_OS_UNKNOWN
#endif

// Warning Suppression Definitions
#if defined(LUNA_CMP_MSVC)
	#define LUNA_DO_PRAGMA(X) _Pragma(#X)
	#define LUNA_DISABLE_ALL_WARNINGS_PUSH				LUNA_DO_PRAGMA(warning(push, 0))
	#define LUNA_DISABLE_WARNING_PUSH					LUNA_DO_PRAGMA(warning(push))
	#define LUNA_DISABLE_WARNING_POP					LUNA_DO_PRAGMA(warning(pop))
	#define LUNA_DISABLE_WARNING(X)						LUNA_DO_PRAGMA(warning(disable : X))

	#define LUNA_DISABLE_WARNING_UNSECURE_FUNCTION		LUNA_DISABLE_WARNING(4996)
	#define LUNA_DISABLE_WARNING_UNUSED_RESULT

#elif defined(LUNA_CMP_GCC) || defined(LUNA_CMP_CLANG)
	#define LUNA_DO_PRAGMA(X) _Pragma(#X)
	#define LUNA_DISABLE_ALL_WARNINGS_PUSH
	#define LUNA_DISABLE_WARNING_PUSH					LUNA_DO_PRAGMA(GCC diagnostic push)
	#define LUNA_DISABLE_WARNING_POP					LUNA_DO_PRAGMA(GCC diagnostic pop)
	#define LUNA_DISABLE_WARNING(id)					LUNA_DO_PRAGMA(GCC diagnostic ignored #id)

	#define LUNA_DISABLE_WARNING_UNSECURE_FUNCTION
	#define LUNA_DISABLE_WARNING_UNUSED_RESULT			LUNA_DISABLE_WARNING(-Wunused-result)
#else
	#define LUNA_DO_PRAGMA(X)
	#define LUNA_DISABLE_WARNING_PUSH
	#define LUNA_DISABLE_WARNING_POP
	#define LUNA_DISABLE_WARNING(id)

	#define LUNA_DISABLE_WARNING_UNSECURE_FUNCTION
	#define LUNA_DISABLE_WARNING_UNUSED_RESULT
#endif

// POSIX function name resolution
#if !defined(LUNA_OS_WINDOWS) && defined(LUNA_CMP_GCC)
	#define printf_s printf
	#define vprintf_s vprintf
	#define fprintf_s fprintf
	#define vfprintf_s vfprintf
	#define memcpy_s(dest, dest_size, src, src_size) memcpy(dest, src, src_size)
	#define memmove_s(dest, dest_size, src, src_size) memmove(dest, src, src_size)
	#define strcat_s(dest, dest_size, src) strcat(dest, src)
	#define strcpy_s(dest, dest_size, src) strcpy(dest, src)
#endif

// Function name refrencing
#if !defined(LUNA_STR_FUNC)
	#if defined(LUNA_CMP_MSVC)
		#define LUNA_STR_FUNC __FUNCTION__
	#elif defined(LUNA_CMP_GCC) || defined(LUNA_CMP_CLANG)
		#define LUNA_STR_FUNC __func__
	#else
		#define LUNA_STR_FUNC "N/A"
	#endif
#endif

#endif