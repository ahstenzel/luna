# SDL
set(SDL_TEST OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_EXAMPLES OFF)
set(SDL_SHARED ON)
add_subdirectory(SDL EXCLUDE_FROM_ALL)
set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:SDL3::SDL3>" CACHE PATH "" FORCE)

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

# vex
add_subdirectory(vex)

# json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)

# glm
set(GLM_BUILD_TESTS OFF CACHE INTERNAL "")
add_subdirectory(glm)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE 
	nlohmann_json::nlohmann_json 
	libcppvex 
	base64 
	libaes 
	liblz4 
	SDL3::SDL3 
	glm::glm
)
set_target_properties(
	libaes 
	liblz4
	base64 
	libcvex
	libcppvex
	SDL_uclibc
	SDL3-shared
	glm
	PROPERTIES FOLDER "Vendor"
)