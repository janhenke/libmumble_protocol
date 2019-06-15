cmake_minimum_required(VERSION 3.14)
project(libmumble_client VERSION 0.0.1)

# compile for C++17 without compiler extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17  )
set(CMAKE_CXX_EXTENSIONS OFF)
# compile for C11 without compiler extensions
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED 11)
set(CMAKE_C_EXTENSIONS OFF)

# always compile position independent
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Use libc++ instead of libstdc++ (Clang only)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
	option(USE_LIBCPP "Use libc++ as C++ standard library" OFF)
	if (USE_LIBCPP)
		add_compile_options(-stdlib=libc++)
	endif ()
endif ()

# Use static boost libraries when possible
option(Boost_USE_STATIC_LIBS "Use static boost libraries when possible" OFF)

add_subdirectory("src")
