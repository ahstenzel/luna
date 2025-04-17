# Build app
set(APP_SOURCE
	main.cpp
	common.cpp
	game.cpp
	resources.cpp
	buffer.cpp
)
set(APP_HEADER
	common.hpp
	game.hpp
	resources.hpp
	buffer.hpp
)
add_executable(${PROJECT_NAME} ${APP_SOURCE} ${APP_HEADER})
source_group("Header Files" FILES ${APP_HEADER})
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/vendor"
)