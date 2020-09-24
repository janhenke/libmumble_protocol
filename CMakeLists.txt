cmake_minimum_required(VERSION 3.13)
project(libmumble_client VERSION 0.0.1)

# compile for C++20 without compiler extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# compile for C11 without compiler extensions
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# always compile position independent
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# hide symbols in shared libraries by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Use libc++ instead of libstdc++ (Clang only)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	option(USE_LIBCPP "Use libc++ as C++ standard library" OFF)
	if (USE_LIBCPP)
		add_compile_options(-stdlib=libc++)
	endif ()
endif ()

# Use static boost libraries when possible
option(Boost_USE_STATIC_LIBS "Use static boost libraries when possible" OFF)
set(BOOST_REQUIRED_VERSION 1.71)
if (${MSVC})
	set(COMPILER_WARNING_OPTIONS /EHsc)
else ()
	set(COMPILER_WARNING_OPTIONS -Wall -Wextra -Wpedantic)
endif ()

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

option(BUILD_EXAMPLE "Build example app" ON)
if (BUILD_EXAMPLE)
	add_subdirectory(example)
endif ()
