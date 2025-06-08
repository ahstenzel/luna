# Include helper functions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(luna_deploy_assets)
include(luna_deploy_shared_libs)
include(luna_include_dirs)

# Add examples
add_subdirectory(hello_sprite)

# Group together in IDE
set_target_properties(
	hello_sprite
	PROPERTIES FOLDER "Examples"
)