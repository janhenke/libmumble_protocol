include(GenerateExportHeader)
include(CheckIPOSupported)
# check for LTO support in the compiler, raise a fatal error is unsupported
check_ipo_supported()

# libmumble_client
add_library(
        mumble_client
        SHARED
        protocol/protocol.h
        protocol/control/declarations.h
        protocol/control/Header.cpp
        protocol/control/Header.h
        protocol/util.cpp
        protocol/util.h
        core/Client.cpp
        core/Client.h
        protocol/control/VersionPacket.h
)

generate_export_header(mumble_client)

set_target_properties(
        mumble_client
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_VISIBILITY_PRESET hidden
        COMPILE_WARNING_AS_ERROR ON
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
        INTERPROCEDURAL_OPTIMIZATION ON
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        SOVERSION 0
)

target_include_directories(
        mumble_client
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(
        mumble_client
        PUBLIC ASIO_NO_DEPRECATED
        PUBLIC BOOST_LOG_DYN_LINK
        PUBLIC BOOST_ASIO_NO_DEPRECATED
        PUBLIC BOOST_ASIO_DISABLE_CONCEPTS
)

# enable extra warnings for our code
target_compile_options(
        mumble_client
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

find_package(asio CONFIG REQUIRED)
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS log log_setup program_options)
find_package(OpenSSL REQUIRED)
find_package(Opus REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(
        mumble_client
        PRIVATE mumble_protocol
        PRIVATE asio::asio
        PRIVATE Boost::boost
        PRIVATE Boost::log
        PRIVATE OpenSSL::SSL
        PRIVATE OpenSSL::Crypto
        PRIVATE Opus::opus
        PUBLIC Threads::Threads
)
