#include "raylib.h"

#define RRES_IMPLEMENTATION
#include "rres.h"              // Required to read rres data chunks

#define LZ4_DISABLE_DEPRECATE_WARNINGS
#define RRES_RAYLIB_IMPLEMENTATION
#define RRES_SUPPORT_COMPRESSION_LZ4
#define RRES_SUPPORT_ENCRYPTION_AES
#define RRES_SUPPORT_ENCRYPTION_XCHACHA20
#include "rres-raylib.h"       // Required to map rres data chunks into raylib structs