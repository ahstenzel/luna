# CMake config
cmake_minimum_required(VERSION 3.26)
project(luna)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)
include(CMakeDependentOption)

# Determine configuration
set(LUNA_SHARED_DEFAULT ON)
set(LUNA_STATIC_DEFAULT ON)
set(LUNA_SHARED_AVAILABLE ON)
set(LUNA_STATIC_AVAILABLE ON)
if(LUNA_SHARED_DEFAULT AND LUNA_STATIC_DEFAULT AND LUNA_SHARED_AVAILABLE)
	if(DEFINED BUILD_SHARED_LIBS)
		# When defined, use BUILD_SHARED_LIBS as default
		if(BUILD_SHARED_LIBS)
			set(LUNA_STATIC_DEFAULT OFF)
		else()
			set(LUNA_SHARED_DEFAULT OFF)
		endif()
	else()
		# Default to just building the static library
		set(LUNA_SHARED_DEFAULT OFF)
	endif()
endif()
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
	set(LUNA_MAINPROJECT ON)
else()
	set(LUNA_MAINPROJECT OFF)
endif()

# Set options
cmake_dependent_option(LUNA_BUILD_SHARED "Build a shared version of the library" ${LUNA_SHARED_DEFAULT} ${LUNA_SHARED_AVAILABLE} OFF)
cmake_dependent_option(LUNA_BUILD_STATIC "Build a static version of the library" ${LUNA_STATIC_DEFAULT} ${LUNA_STATIC_AVAILABLE} OFF)
option(LUNA_EXAMPLES "Build the examples directory" ${LUNA_MAINPROJECT})
if(NOT(LUNA_BUILD_SHARED OR LUNA_BUILD_STATIC))
	message(FATAL_ERROR "LUNA_BUILD_SHARED and LUNA_BUILD_STATIC cannot both be disabled")
endif()

# Add components
unset(LUNA_RUNTIME_SHARED CACHE)
add_subdirectory(vendor)
add_subdirectory(external)
add_subdirectory(tools)
add_subdirectory(src)
if (${LUNA_EXAMPLES})
	add_subdirectory(examples)
endif()