#pragma once

// =========================================================================== Includes
// Common includes
#include <stdexcept>
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <functional>
#include <chrono>
#include <filesystem>
#include <variant>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_map>
#include <unordered_set>

// SDL includes
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

// External libraries
#define LZ4_HEAPMODE 1
#include <lz4/lz4.h>
#define CBC 1
#define ECB 0
#define CTR 0
#include <aes/aes.h>
#include <libbase64.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

// Luna includes
#include <luna/detail/std/platform.hpp>
#include <luna/detail/std/itsort.hpp>
#include <luna/detail/std/buffer.hpp>
#include <luna/detail/std/sorted_list.hpp>

namespace luna {

// =========================================================================== Helper Functions
/// <summary>
/// Round the number up to the next multiple.
/// </summary>
/// <param name="num">Number to round</param>
/// <param name="multiple">Multiple to round to</param>
/// <returns></returns>
LUNA_API std::uint64_t RoundUp(std::uint64_t num, std::uint64_t multiple);

/// <summary>
/// Get the next power of 2 higher than the given number.
/// </summary>
/// <param name="num"></param>
/// <returns></returns>
LUNA_API std::uint64_t NextPow2(std::uint64_t num);

/// <summary>
/// Wrap the given value around the range [lower, upper), handling negative values.
/// </summary>
/// <param name="val">Value to wrap</param>
/// <param name="lower">Lower bound (inclusive)</param>
/// <param name="upper">Upper bound (exclusive)</param>
/// <returns></returns>
LUNA_API std::int32_t Wrap(std::int32_t val, std::int32_t lower, std::int32_t upper);

/// <summary>
/// Tokenize a string using the given delimiter.
/// </summary>
/// <param name="str">Input string</param>
/// <param name="delim">Seperator</param>
/// <returns>List of strings</returns>
LUNA_API std::vector<std::string> StringSplit(const std::string& str, const std::string& delim);

/// <summary>
/// Check if the two version strings match. If 'exact' is false, only match the major & minor revisions.
/// </summary>
/// <param name="v1">Version string 1</param>
/// <param name="v2">Version string 2</param>
/// <param name="exact">Require patch revision to match as well</param>
/// <returns>True if correct revisions match</returns>
LUNA_API bool VersionStringMatch(const std::string& v1, const std::string& v2, bool exact = false);

/// <summary>
/// Calculate the CRC32 value for the given data.
/// </summary>
/// <param name="data">Data buffer</param>
/// <param name="length">Buffer length</param>
/// <param name="previousCRC">Previous CRC value (only if data is being appended to an existing CRC value)</param>
/// <returns>CRC value</returns>
LUNA_API std::uint32_t Crc32Calculate(const void* data, std::size_t length, std::uint32_t previousCRC = 0);

/// <summary>
/// Convert the given integer color to normalized floating point color.
/// </summary>
/// <param name="color"></param>
/// <returns></returns>
LUNA_API SDL_FColor ConvertToFColor(SDL_Color color);

// =========================================================================== Global Definitions
constexpr SDL_Color LunaColorClear = { 0, 0, 0, 255 };
constexpr SDL_Color LunaColorWhite = {255, 255, 255, 255};
constexpr SDL_Color LunaColorLightGray = { 170, 170, 170, 255 };
constexpr SDL_Color LunaColorDarkGray = { 85, 85, 85, 255 };
constexpr SDL_Color LunaColorBlack = { 0, 0, 0, 255 };
constexpr SDL_Color LunaColorRed = { 255, 0, 0, 255 };
constexpr SDL_Color LunaColorOrange = { 255, 170, 0, 255 };
constexpr SDL_Color LunaColorYellow = { 255, 255, 0, 255 };
constexpr SDL_Color LunaColorLightGreen = { 170, 255, 0, 255 };
constexpr SDL_Color LunaColorGreen = { 0, 255, 0, 255 };
constexpr SDL_Color LunaColorCyan = { 0, 170, 170, 255 };
constexpr SDL_Color LunaColorLightBlue = { 85, 255, 255, 255 };
constexpr SDL_Color LunaColorBlue = { 0, 0, 255, 255 };
constexpr SDL_Color LunaColorMagenta = { 170, 0, 255, 255 };
constexpr SDL_Color LunaColorPurple = { 85, 0, 255, 255 };
constexpr SDL_Color LunaColorPink = { 255, 170, 255, 255 };
constexpr SDL_Color LunaColorBrown = { 100, 50, 0, 255 };

constexpr double PHI_D = 1.6180339887498949025257388711906969547271728515625;
constexpr float PHI_F = 1.61803400516510009765625f;
constexpr double SQRT2_D = 1.4142135623730951454746218587388284504413604736328125;
constexpr float SQRT2_F = 1.41421353816986083984375f;

#ifdef SDL_PI_D
constexpr double PI_D = SDL_PI_D;
#else
constexpr double PI_D = 3.141592653589793115997963468544185161590576171875;
#endif

#ifdef SDL_PI_F
constexpr float PI_F = SDL_PI_F;
#else
constexpr float PI_F = 3.1415927410125732421875f;
#endif

constexpr double PI_2_D = PI_D / 2.0;
constexpr float PI_2_F = PI_F / 2.0f;
constexpr double TAU_D = 2.0 * PI_D;
constexpr float TAU_F = 2.0f * PI_F;

/*
constexpr double PI_2_D = 1.5707963267948965579989817342720925807952880859375;
constexpr float PI_2_F = 1.57079637050628662109375f;
constexpr double TAU_D = 6.28318530717958623199592693708837032318115234375;
constexpr float TAU_F = 6.283185482025146484375f;
*/

} // luna