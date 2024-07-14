# Example project: Simple
project(example_simple)

# Add source files
file (GLOB game_src
	"${PROJECT_SOURCE_DIR}/src/main.c"
)
file (GLOB game_rsc
	"${PROJECT_SOURCE_DIR}/rsc/settings.ini"
	"${PROJECT_SOURCE_DIR}/rsc/example_simple.rres"
)

# Set output
set(OUTPUT_TREE "bin/examples/simple/$<IF:$<CONFIG:Debug>,Debug,Release>")
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/${OUTPUT_TREE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

# Build binary
add_executable(example_simple ${game_src})

# Add dependencies
set(LIB_DIR    "${CMAKE_SOURCE_DIR}/bin/luna/$<IF:$<CONFIG:Debug>,Debug,Release>")
set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/lib/${OS}/${GEN}/$<IF:$<CONFIG:Debug>,Debug,Release>")
target_include_directories(example_simple PUBLIC "${CMAKE_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src")
target_link_directories(example_simple PUBLIC ${LIB_DIR})
target_link_libraries(example_simple PUBLIC luna)

# Deploy runtime files
set(DEPLOY_FILES
	${game_rsc}
	"$<IF:$<CONFIG:Debug>,${LIB_DIR}/luna.pdb,>"
	"$<IF:$<CONFIG:Debug>,${RAYLIB_DIR}/raylib.pdb,>"
)
add_custom_command(TARGET example_simple POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${DEPLOY_FILES} "${OUTPUT_DIR}"
)

# Platform configuration
if (OS STREQUAL "windows" AND GEN STREQUAL "msvc2022")
	set_target_properties(
		example_simple PROPERTIES 
		LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT /SUBSYSTEM:CONSOLE"
		LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
	)
	target_compile_definitions(example_simple PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()