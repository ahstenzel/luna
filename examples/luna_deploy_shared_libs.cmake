function(luna_deploy_shared_libs PROJECT DEST_DIR)
	if(CMAKE_SYSTEM_NAME MATCHES "Windows")
		if (MSVC)
			set(RUNTIME_SHARED_DIR "${DEST_DIR}/$<CONFIG>")
		else()
			set(RUNTIME_SHARED_DIR "${DEST_DIR}")
		endif()
		add_custom_command(
			TARGET ${PROJECT} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				"${LUNA_RUNTIME_SHARED}"
				"${RUNTIME_SHARED_DIR}"
			COMMAND_EXPAND_LISTS
		)
	endif()
endfunction()