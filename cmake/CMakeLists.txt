cmake_minimum_required(VERSION 3.2)
project(Megrez VERSION 0.0.7 LANGUAGES C CXX)

# Enables link_directories() treat paths relative
# to the source dir.
if(POLICY CMP0015)
	cmake_policy(SET CMP0015 NEW)
endif(POLICY CMP0015)

# Enables project() command manages VERSION variables.
if(POLICY CMP0048)
	cmake_policy(SET CMP0048 NEW)
endif(POLICY CMP0048)

# Create the library target
set(project_library_target_name ${PROJECT_NAME})
set(PACKAGE_NAME Megrez)
add_library(${project_library_target_name} INTERFACE)

#####
# Change the default build type from Debug to Release, while still
# supporting overriding the build type.
#
# The CACHE STRING logic here and elsewhere is needed to force CMake
# to pay attention to the value of these variables.
if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type specified; defaulting to CMAKE_BUILD_TYPE=Release.")
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
else(NOT CMAKE_BUILD_TYPE)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		message("==========================================================================================")
		message(STATUS "Build type: Debug. Performance will be terrible!")
		message(STATUS "Add -DCMAKE_BUILD_TYPE=Release to the CMake command line to get an optimized build.")
		message("==========================================================================================")
	endif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif(NOT CMAKE_BUILD_TYPE)

# Setup the compiler options
# set c++ standard to c++11.
# Note: not working on CMake 2.8. We assume that user has
#       a compiler with C++11 support.

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "C++11 support has been enabled by default.")

option(BUILD_LIB         "Set to ON to build the library"     OFF)
option(BUILD_COMPILER    "Set to ON to build the compiler"    OFF)
option(BUILD_TESTS       "Set to ON to build the tests"       OFF)
option(BUILD_EXAMPLES    "Set to ON to build the examples"    OFF)
option(BUILD_BENCHMARKS  "Set to ON to build the benchmarks"  OFF)
option(BUILD_SHARED_LIB  "Set to ON to build shared library"  OFF)

set(MEGREZ_DIR ../Megrez)
include(functions.cmake)
include(megrezlite.cmake)
include(libmegrezc.cmake)