function(luna_include_dirs PROJECT)
	target_include_directories(${PROJECT} PRIVATE
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_SOURCE_DIR}/vendor"
		"${PROJECT_SOURCE_DIR}/vendor/SDL/include"
		"${PROJECT_SOURCE_DIR}/vendor/base64/include"
		"${PROJECT_SOURCE_DIR}/vendor/json/include"
		"${LUNA_SDL_SHADERCROSS_DIR}/include"
	)
endfunction()