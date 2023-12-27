cmake_minimum_required(VERSION 3.27)
project(libmumble_client VERSION 0.0.1)

# Use libc++ instead of libstdc++ (Clang only)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	option(USE_LIBCPP "Use libc++ as C++ standard library" OFF)
	if (USE_LIBCPP)
		add_compile_options(-stdlib=libc++)
	endif ()
endif ()

include(GNUInstallDirs)

# Use static boost libraries when possible
option(Boost_USE_STATIC_LIBS "Use static boost libraries when possible" OFF)
set(BOOST_REQUIRED_VERSION 1.71)

add_subdirectory(mumble_protocol)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)

option(BUILD_EXAMPLE "Build example app" ON)
if (BUILD_EXAMPLE)
	add_subdirectory(example)
endif ()
