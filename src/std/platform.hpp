// Engine version
#define LUNA_VERSION_MAJOR 1
#define LUNA_VERSION_MINOR 0
#define LUNA_VERSION_PATCH 0

// Resource file version
#define APOLLO_VERSION_MAJOR 1
#define APOLLO_VERSION_MINOR 0
#define APOLLO_VERSION_PATCH 0

// Version strings
#define STRINGIFY_(X) #X
#define STRINGIFY(X) STRINGIFY_(X)
#define MAKE_VERSION_STR(major, minor, patch) (STRINGIFY(major) "." STRINGIFY(minor) "." STRINGIFY(patch))

#define LUNA_VERSION_STR MAKE_VERSION_STR(LUNA_VERSION_MAJOR, LUNA_VERSION_MINOR, LUNA_VERSION_PATCH)
#define APOLLO_VERSION_STR MAKE_VERSION_STR(APOLLO_VERSION_MAJOR, APOLLO_VERSION_MINOR, APOLLO_VERSION_PATCH)

// Detect compiler
#if defined(__clang__)
# define LUNA_CMP_CLANG
#elif (defined(__GNUC__) || defined(__GNUG__)) && !defined(__INTEL_COMPILER)
# define LUNA_CMP_GCC
#elif defined(_MSC_VER)
# define LUNA_CMP_MSVC
#elif
# define LUNA_CMP_UNKNOWN
#endif

// Detect architecture
#if defined(__arm__) || defined(__thumb__) ||\
    defined(_M_ARM)  || defined(_M_ARMT)
# define LUNA_ARCH_ARM
#elif defined(__aarch64__) || defined (_M_ARM64)
# define LUNA_ARCH_ARM64
#elif defined(i386) || defined(__i386) || defined(__i386__) ||\
      defined(i486) || defined(__i486) || defined(__i486__) ||\
      defined(i586) || defined(__i586) || defined(__i586__) ||\
      defined(i686) || defined(__i686) || defined(__i686__) ||\
      defined(_M_IX86) || defined(_M_X86) || defined(_X86_)
# define LUNA_ARCH_X86
#elif defined(__x86_64) || defined(__x86_64__) ||\
      defined(__amd64) || defined(__amd64__) ||\
      defined(_M_X64) || defined(_M_AMD64)
# define LUNA_ARCH_X64
#else
# define LUNA_ARCH_UNKNOWN
#endif

// SIMD Definitions
//#define LUNA_DISABLE_SIMD
#if (defined(LUNA_ARCH_X86) || defined(LUNA_ARCH_X64)) && !defined(LUNA_DISABLE_SIMD) && !defined(LUNA_CMP_UNKNOWN)
# define LUNA_SIMD_AVX

#elif (defined(LUNA_ARCH_ARM) || defined(LUNA_ARCH_ARM64)) && !defined(LUNA_DISABLE_SIMD) && !defined(LUNA_CMP_UNKNOWN)
# define LUNA_SIMD_NEON

#else
# define LUNA_SIMD_NONE

#endif