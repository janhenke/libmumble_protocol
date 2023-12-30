cmake_minimum_required(VERSION 3.27)
project(libmumble_protocol VERSION 0.0.1)

# Use libc++ instead of libstdc++ (Clang only)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	option(USE_LIBCPP "Use libc++ as C++ standard library" OFF)
	if (USE_LIBCPP)
		add_compile_options(-stdlib=libc++)
	endif ()
endif ()

# Place all output binaries into the root of the build directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(BOOST_REQUIRED_VERSION 1.83)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CheckIPOSupported)
# check for LTO support in the compiler
check_ipo_supported(RESULT LTO_SUPPORTED)

option(BUILD_CLIENT "Build the client library" ON)
option(BUILD_SERVER "Build the server library" ON)
option(BUILD_TEST "Build the test executables" ON)

if (${BUILD_TEST})
	enable_testing()
	find_package(Catch2 3 REQUIRED)
	include(CTest)
	include(Catch)
endif ()

add_subdirectory(common)

#add_subdirectory(src)

#enable_testing()
#add_subdirectory(test)

#option(BUILD_EXAMPLE "Build example app" ON)
#if (BUILD_EXAMPLE)
#	add_subdirectory(example)
#endif ()
