function(luna_deploy_assets PROJECT DEST_DIR ASSETS)
	add_custom_command(
		TARGET ${PROJECT} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			"${ASSETS}"
			"${DEST_DIR}"
		COMMAND_EXPAND_LISTS
	)
	if (MSVC)
		add_custom_command(
			TARGET ${PROJECT} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				"${ASSETS}"
				"${DEST_DIR}/$<CONFIG>"
			COMMAND_EXPAND_LISTS
		)
	endif()
endfunction()