cmake_minimum_required(VERSION 3.13)
project(luna)

# Add source files
file(GLOB ext_src
	"src/external/ini.c"
)
file(GLOB raylib_src 
	"src/raylib/raygui.c" 
	"src/raylib/rres.c"
)
file(GLOB luna_src 
	"src/luna/std/debug.c"
	"src/luna/std/deque.c"
	"src/luna/std/free_list.c"
	"src/luna/std/priority_queue.c"
	"src/luna/std/queue.c"
	"src/luna/std/stack.c"
	"src/luna/std/unordered_map_str.c"
	"src/luna/std/unordered_map.c"
	"src/luna/std/utilities.c"
	"src/luna/std/vector.c"
	"src/luna/collision.c"
	"src/luna/game.c"
	"src/luna/resource.c"
	"src/luna/scene.c"
	"src/luna/sprite.c"
	"src/luna/input.c"
	"src/luna/tile.c"
	"src/luna/camera.c"
	"src/luna/error.c"
)

# Set output
set(OUTPUT_TREE "bin/luna/$<IF:$<CONFIG:Debug>,Debug,Release>")
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/${OUTPUT_TREE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

# Build binary
add_library(luna ${luna_src} ${raylib_src} ${ext_src})

# Add dependencies
set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib/${OS}/${GEN}/$<IF:$<CONFIG:Debug>,Debug,Release>")
target_include_directories(luna PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_directories(luna PUBLIC ${LIB_DIR})
target_link_libraries(luna PUBLIC raylib opengl32 gdi32 winmm)

# Enforce standards
target_compile_features(luna PRIVATE c_std_11)

# Build examples
add_subdirectory(examples)