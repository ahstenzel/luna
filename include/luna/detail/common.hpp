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
#include <vector>
#include <stack>
#include <unordered_map>

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

} // luna