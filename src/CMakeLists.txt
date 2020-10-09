include(GenerateExportHeader)
# check for LTO support in the compiler
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

add_subdirectory(packet)

# libmumble_client
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS log log_setup)
find_package(Threads REQUIRED)
find_package(OpenSSL)

find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h)
find_library(OPUS_LIBRARY NAMES opus)

add_library(mumble_client
            SHARED
            protocol/voice.cpp protocol/voice.hpp
            core.cpp core.hpp)
set_target_properties(mumble_client PROPERTIES
                      VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
                      SOVERSION 0)
generate_export_header(mumble_client)
target_include_directories(mumble_client
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                           PRIVATE ${OPUS_INCLUDE_DIR})
target_compile_definitions(mumble_client
                           PRIVATE BOOST_LOG_DYN_LINK
                           PRIVATE BOOST_ASIO_NO_DEPRECATED
                           PRIVATE BOOST_ASIO_DISABLE_CONCEPTS)
# enable extra warnings for our code
target_compile_options(mumble_client PRIVATE ${COMPILER_WARNING_OPTIONS})
target_link_libraries(mumble_client
                      PRIVATE mumble_packet
                      PRIVATE Threads::Threads
                      PRIVATE OpenSSL::SSL
                      PRIVATE OpenSSL::Crypto
                      PRIVATE Boost::boost
                      PRIVATE Boost::log
                      PRIVATE ${OPUS_LIBRARY})
if (lto_supported)
	set_property(TARGET mumble_client PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()