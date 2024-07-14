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
file (GLOB game_src
	"src/main.c"
	"src/scenes.c"
)
file (GLOB game_rsc
	"rsc/settings.ini"
	"rsc/luna.rres"
)

# Set output
set(OUTPUT_TREE "bin/$<IF:$<CONFIG:Debug>,Debug,Release>")
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/${OUTPUT_TREE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

# Build binary
add_executable(luna ${game_src} ${luna_src} ${raylib_src} ${ext_src})

# Add dependencies
set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib/${OS}/${GEN}/$<IF:$<CONFIG:Debug>,Debug,Release>")
target_include_directories(luna PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_link_directories(luna PUBLIC ${LIB_DIR})
target_link_libraries(luna PUBLIC raylib opengl32 gdi32 winmm)

# Copy resource files
set(DEPLOY_FILES
	${game_rsc}
	"$<IF:$<CONFIG:Debug>,${LIB_DIR}/raylib.pdb,>"
)
add_custom_command(TARGET luna POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${DEPLOY_FILES} "${OUTPUT_DIR}"
)

# Platform configuration
if (OS STREQUAL "windows" AND GEN STREQUAL "msvc2022")
	set_target_properties(
		luna PROPERTIES 
		LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT /SUBSYSTEM:CONSOLE"
		LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
	)
	target_compile_definitions(luna PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Enforce standards
target_compile_features(luna PRIVATE c_std_11)