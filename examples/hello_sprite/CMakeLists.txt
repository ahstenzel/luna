add_executable(hello_sprite main.cpp)
target_link_libraries(hello_sprite PRIVATE libluna vendor external)
luna_include_dirs(hello_sprite)
luna_deploy_shared_libs(hello_sprite ${CMAKE_CURRENT_BINARY_DIR})