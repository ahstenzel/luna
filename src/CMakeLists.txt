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
	"${PROJECT_SOURCE_DIR}/include/luna/detail/shader/shader_encoded.hpp"
)
set(APP_HEADER_STD
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/platform.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/buffer.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/sorted_list.hpp"
	"${PROJECT_SOURCE_DIR}/include/luna/detail/std/itsort.hpp"
)
if(LUNA_SHARED)
	add_library(luna SHARED ${APP_SOURCE} ${APP_HEADER} ${APP_HEADER_STD})
	set(LUNA_RUNTIME_SHARED "${LUNA_RUNTIME_SHARED};$<TARGET_FILE:luna>" CACHE PATH "" FORCE)
elseif(LUNA_STATIC)
	add_library(luna STATIC ${APP_SOURCE} ${APP_HEADER} ${APP_HEADER_STD})
endif()
source_group("Header Files" FILES ${APP_HEADER})
source_group("Header Files/std" FILES ${APP_HEADER_STD})
target_link_libraries(luna PRIVATE vendor external)
add_dependencies(luna luna_shader)

# Add include directories
target_include_directories(luna PRIVATE
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/vendor"
	"${PROJECT_SOURCE_DIR}/vendor/SDL/include"
	"${PROJECT_SOURCE_DIR}/vendor/base64/include"
	"${PROJECT_SOURCE_DIR}/vendor/json/include"
	"${PROJECT_SOURCE_DIR}/vendor/glm"
	"${LUNA_SDL_SHADERCROSS_DIR}/include"
)