# SDL
set(SDL_TEST OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_EXAMPLES OFF)
set(SDL_SHARED ON)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:SDL3::SDL3>" CACHE PATH "" FORCE)

# SDL_ttf
set(SDLTTF_VENDORED ON)
add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)
set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:SDL3_ttf::SDL3_ttf>" CACHE PATH "" FORCE)

# SDL_image
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_TIF OFF)
add_subdirectory(SDL_image EXCLUDE_FROM_ALL)
set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:SDL3_image::SDL3_image>" CACHE PATH "" FORCE)

# AES
add_subdirectory(aes)

# LZ4
add_subdirectory(lz4)

# base64
set(BASE64_WITH_SSSE3 ON)
set(BASE64_WITH_SSE41 ON)
set(BASE64_WITH_SSE42 ON)
set(BASE64_WITH_AVX ON)
set(BASE64_WITH_AVX2 ON)
set(BASE64_WITH_AVX512 ON)
set(BASE64_WITH_NEON64 ON)
add_subdirectory(base64)
set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:base64>" CACHE PATH "" FORCE)

# vex
add_subdirectory(vex)

# json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

# glm
set(GLM_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLM_ENABLE_FAST_MATH ON CACHE INTERNAL "")
set(GLM_ENABLE_SIMD_AVX2 ON CACHE INTERNAL "")
set(GLM_ENABLE_SIMD_NEON ON CACHE INTERNAL "")
add_subdirectory(glm)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE 
	nlohmann_json::nlohmann_json 
	libcppvex 
	base64 
	libaes 
	liblz4 
	SDL3::SDL3 
	SDL3_ttf::SDL3_ttf
	SDL3_image::SDL3_image
)
set_target_properties(
	libaes 
	liblz4
	base64 
	libcvex
	libcppvex
	SDL_uclibc
	SDL3-shared
	SDL3_ttf-shared
	plutosvg
	plutovg
	harfbuzz
	freetype
	SDL3_image-shared
	sharpyuv
	webp
	webpdecode
	webpdemux
	webpdsp
	webpencode
	webputils
	glm
	PROPERTIES FOLDER "Vendor"
)