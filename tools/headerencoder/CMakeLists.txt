add_executable(headerencoder main.cpp)
target_include_directories(headerencoder PRIVATE
	"${PROJECT_SOURCE_DIR}/vendor/"
	"${PROJECT_SOURCE_DIR}/vendor/base64/include"
	"${PROJECT_SOURCE_DIR}/vendor/json/include"
)
target_link_libraries(headerencoder libcppvex base64)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	if (MSVC)
		set(RUNTIME_SHARED_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
	else()
		set(RUNTIME_SHARED_DIR "${CMAKE_CURRENT_BINARY_DIR}")
	endif()
	set(BASE64_RUNTIME_SHARED "$<TARGET_FILE:base64>")
	add_custom_command(
		TARGET headerencoder POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			"${BASE64_RUNTIME_SHARED}"
			"${RUNTIME_SHARED_DIR}"
		COMMAND_EXPAND_LISTS
	)
endif()