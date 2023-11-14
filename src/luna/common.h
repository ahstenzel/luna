#ifndef LUNA_COMMON_H
#define LUNA_COMMON_H
/**
 * common.h
 * Common defines & includes for the whole game.
*/

//----------------------------------------------------------------------------------
// Header inclusions
//----------------------------------------------------------------------------------



// Standard includes
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <memory.h>

// OS includes
#include "luna/platform.h"
#ifdef LUNA_OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

// Raylib includes
#include "raylib/raylib.h"
#include "raylib/raygui.h"
#include "raylib/rres.h"
#include "raylib/rres-raylib.h"
#include "raylib/raymath.h"

// Luna includes
#include "luna/std/utilities.h"
#include "luna/std/debug.h"
#include "luna/std/vector.h"
#include "luna/std/stack.h"
#include "luna/std/unordered_map.h"
#include "luna/std/unordered_map_str.h"
#include "luna/std/queue.h"
#include "luna/std/priority_queue.h"
#include "luna/std/free_list.h"
#include "luna/std/deque.h"

#endif