# Build shaders
add_subdirectory(shader)

# Build app
set(APP_SOURCE
	main.cpp
	common.cpp
	game.cpp
	resources.cpp
	shader.cpp
	sprite.cpp
	render.cpp
	shader/out/shader_encoded.cpp
)
set(APP_HEADER
	common.hpp
	game.hpp
	resources.hpp
	shader.hpp
	sprite.hpp
	render.hpp
	shader/out/shader_encoded.hpp
)
set(APP_HEADER_STD
	std/platform.hpp
	std/buffer.hpp
	std/sorted_list.hpp
	std/itsort.hpp
)
add_executable(${PROJECT_NAME} ${APP_SOURCE} ${APP_HEADER} ${APP_HEADER_STD})
source_group("Header Files" FILES ${APP_HEADER})
source_group("Header Files/std" FILES ${APP_HEADER_STD})
target_link_libraries(${PROJECT_NAME} PRIVATE vendor external)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_shader)

# Add include directories
message("LUNA_SDL_SHADERCROSS_DIR: ${LUNA_SDL_SHADERCROSS_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/vendor"
	"${PROJECT_SOURCE_DIR}/vendor/base64/include"
	"${PROJECT_SOURCE_DIR}/vendor/json/include"
	"${LUNA_SDL_SHADERCROSS_DIR}/include"
)

# Add shared libraries
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if (MSVC)
		set(SHARED_LIBS_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/$<CONFIG>")
	else()
		set(SHARED_LIBS_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
	endif()
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			"${LUNA_SDL_SHADERCROSS_SHARED_LIBS}"
			"${SHARED_LIBS_DIR}"
		COMMAND_EXPAND_LISTS
	)
endif()