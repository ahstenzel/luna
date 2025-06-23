# Collect source files
set(APP_SHADER_FRAG_SRC
	"${PROJECT_SOURCE_DIR}/src/shader/SpriteBatch.frag.hlsl"
	"${PROJECT_SOURCE_DIR}/src/shader/PrimitiveBatch.frag.hlsl"
)
set(APP_SHADER_VERT_SRC
	"${PROJECT_SOURCE_DIR}/src/shader/SpriteBatch.vert.hlsl"
	"${PROJECT_SOURCE_DIR}/src/shader/PrimitiveBatch.vert.hlsl"
)
set(APP_SHADER_SRC ${APP_SHADER_FRAG_SRC} ${APP_SHADER_VERT_SRC})

# Mark source file properties
set_source_files_properties(${APP_SHADER_FRAG_SRC} PROPERTIES ShaderType "frag")
set_source_files_properties(${APP_SHADER_VERT_SRC} PROPERTIES ShaderType "vert")
if (MSVC)
	# Prevent VS from trying to compile shaders
	set_source_files_properties(${APP_SHADER_SRC} PROPERTIES VS_SETTINGS "ExcludedFromBuild=true")
endif()

# Generate metadata
foreach(FILE ${APP_SHADER_SRC})
	get_filename_component(FILE_BASENAME ${FILE} NAME_WLE)
	set(FILE_OUT "${PROJECT_SOURCE_DIR}/src/shader/${FILE_BASENAME}.json")
	set(APP_SHATER_META ${APP_SHATER_META} ${FILE_OUT})
	add_custom_command(
		OUTPUT ${FILE_OUT}
		COMMAND ${LUNA_SDL_SHADERCROSS} ${FILE} -o ${FILE_OUT}
	)
endforeach()

# Encode into header
set(APP_SHADER_OUTPUT_HEADER "${PROJECT_SOURCE_DIR}/src/shader/shader_encoded.hpp")
set(APP_SHADER_OUTPUT_SOURCE "${PROJECT_SOURCE_DIR}/src/shader/shader_encoded.cpp")
add_custom_command(
	OUTPUT ${APP_SHADER_OUTPUT_HEADER}
	DEPENDS ${APP_SHADER_SRC} ${APP_SHATER_META}
	COMMAND headerencoder -i ${APP_SHADER_SRC} -o ${APP_SHADER_OUTPUT_HEADER} -s
)
add_custom_target(libluna_shader ALL
	SOURCES ${APP_SHADER_SRC}
	DEPENDS ${APP_SHADER_OUTPUT_HEADER}
)
add_custom_command(
	TARGET libluna_shader POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E rename
		"${APP_SHADER_OUTPUT_HEADER}"
		"${PROJECT_SOURCE_DIR}/include/luna/detail/shader/shader_encoded.hpp"
)
source_group("Source Files" FILES ${APP_SHADER_SRC})
unset(LUNA_SHADER_LANGUAGE CACHE)