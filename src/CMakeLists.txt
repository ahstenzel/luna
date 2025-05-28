# Build shaders
add_subdirectory(shader)

# Build app
set(APP_SOURCE
	"${PROJECT_SOURCE_DIR}/src/common.cpp"
	"${PROJECT_SOURCE_DIR}/src/game.cpp"
	"${PROJECT_SOURCE_DIR}/src/resources.cpp"
	"${PROJECT_SOURCE_DIR}/src/shader.cpp"
	"${PROJECT_SOURCE_DIR}/src/sprite.cpp"
	"${PROJECT_SOURCE_DIR}/src/render.cpp"
	"${PROJECT_SOURCE_DIR}/src/camera.cpp"
	"${PROJECT_SOURCE_DIR}/src/room.cpp"
	"${PROJECT_SOURCE_DIR}/src/shader/shader_encoded.cpp"
)
set(APP_HEADER
	"${PROJECT_SOURCE_DIR}/include/luna/detail/common.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/game.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/resources.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/shader.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/sprite.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/render.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/camera.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/room.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/shader/shader_encoded.hpp"
)
set(APP_HEADER_STD
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/platform.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/buffer.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/sorted_list.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/itsort.hpp"
)
if(LUNA_BUILD_SHARED)
	add_library(libluna SHARED ${APP_SOURCE} ${APP_HEADER} ${APP_HEADER_STD})
	target_compile_definitions(libluna PRIVATE LUNA_BUILD_SHARED)
	target_compile_definitions(libluna PRIVATE LUNA_SHARED_EXPORT)
	set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:libluna>" CACHE PATH "" FORCE)
elseif(LUNA_BUILD_STATIC)
	add_library(libluna STATIC ${APP_SOURCE} ${APP_HEADER} ${APP_HEADER_STD})
endif()
if(MSVC)
	target_compile_definitions(libluna PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
source_group("Header Files" FILES ${APP_HEADER})
source_group("Header Files/std" FILES ${APP_HEADER_STD})
target_link_libraries(libluna PRIVATE vendor external)
add_dependencies(libluna libluna_shader)

# Add include directories
target_include_directories(libluna PRIVATE
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/vendor"
	"${PROJECT_SOURCE_DIR}/vendor/SDL/include"
	"${PROJECT_SOURCE_DIR}/vendor/base64/include"
	"${PROJECT_SOURCE_DIR}/vendor/json/include"
	"${PROJECT_SOURCE_DIR}/vendor/glm"
	"${LUNA_SDL_SHADERCROSS_DIR}/include"
)