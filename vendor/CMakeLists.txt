# SDL
set(SDL_TEST OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_EXAMPLES OFF)
add_subdirectory(SDL EXCLUDE_FROM_ALL)

# SDL_shadercross
set(SDL3_DIR "${CMAKE_CURRENT_BINARY_DIR}/SDL")
set(FPHSA_NAME_MISMATCHED ON)
set(SDLSHADERCROSS_VENDORED ON)
set(SDLSHADERCROSS_CLI OFF)
set(SDLSHADERCROSS_INSTALL_MAN OFF)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
cmake_policy(PUSH)
cmake_policy(SET CMP0024 OLD)
add_subdirectory(SDL_shadercross EXCLUDE_FROM_ALL)
cmake_policy(POP)

# AES
add_subdirectory(aes EXCLUDE_FROM_ALL)

# LZ4
add_subdirectory(lz4 EXCLUDE_FROM_ALL)

add_library(vendor INTERFACE)
target_link_libraries(vendor INTERFACE libaes liblz4 SDL3_shadercross::SDL3_shadercross SDL3::SDL3)