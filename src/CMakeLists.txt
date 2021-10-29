include(GenerateExportHeader)
# check for LTO support in the compiler
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

# libmumble_client
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS log log_setup program_options)
find_package(Threads REQUIRED)
find_package(OpenSSL)

find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h)
find_library(OPUS_LIBRARY NAMES opus)

add_library(
        mumble_client SHARED
        protocol/protocol.h
        protocol/control/declarations.h
        protocol/control/Header.cpp
        protocol/control/Header.h
        protocol/util.cpp
        protocol/util.h
        core/Client.cpp
        core/Client.h
        protocol/control/VersionPacket.h)

generate_export_header(mumble_client)

set_target_properties(
        mumble_client PROPERTIES
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        SOVERSION 0
)

target_include_directories(
        mumble_client
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${OPUS_INCLUDE_DIR}
)

target_compile_definitions(
        mumble_client
        PUBLIC ASIO_NO_DEPRECATED
        PUBLIC BOOST_LOG_DYN_LINK
        PUBLIC BOOST_ASIO_NO_DEPRECATED
        PUBLIC BOOST_ASIO_DISABLE_CONCEPTS
)

# enable extra warnings for our code
target_compile_options(mumble_client PRIVATE ${COMPILER_WARNING_OPTIONS})

target_link_libraries(
        mumble_client
        PRIVATE mumble_protocol
        PUBLIC ${CMAKE_THREAD_LIBS_INIT}
        PUBLIC OpenSSL::SSL
        PRIVATE OpenSSL::Crypto
        PRIVATE Boost::boost
        PRIVATE Boost::log
        PRIVATE ${OPUS_LIBRARY}
)

if (lto_supported)
    set_property(TARGET mumble_client PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()