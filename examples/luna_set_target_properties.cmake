function(luna_set_target_properties PROJECT)
	if(CMAKE_SYSTEM_NAME MATCHES "Windows")
		if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
			set_target_properties(
				${PROJECT} PROPERTIES 
				LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT /SUBSYSTEM:CONSOLE"
				LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
			)
			target_compile_definitions(${PROJECT} PRIVATE _CRT_SECURE_NO_WARNINGS)
		elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
			set_target_properties(
				${PROJECT} PROPERTIES 
				LINK_FLAGS_DEBUG "-Wl,/subsystem:console"
				LINK_FLAGS_RELEASE "-Wl,/subsystem:windows,/entry:mainCRTStartup"
			)
		else()
			set_target_properties(
				${PROJECT} PROPERTIES 
				LINK_FLAGS_DEBUG "-Wl,-subsystem,console"
				LINK_FLAGS_RELEASE "-Wl,-subsystem,windows"
			)
		endif()
	endif()
endfunction()