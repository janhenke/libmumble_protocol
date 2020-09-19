include(GenerateExportHeader)
# check for LTO support in the compiler
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

# libmumble_client_protocol
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(MUMBLE_PROTOBUF_SOURCE_FILES MUMBLE_PROTOBUF_HEADER_FILES protocol/Mumble.proto)
add_library(mumble_client_protocol SHARED ${MUMBLE_PROTOBUF_SOURCE_FILES} ${MUMBLE_PROTOBUF_HEADER_FILES})
set_target_properties(mumble_client_protocol PROPERTIES
                      VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
                      SOVERSION 0
                      WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(mumble_client_protocol PUBLIC ${Protobuf_INCLUDE_DIRS})
target_link_libraries(mumble_client_protocol
                      PRIVATE Threads::Threads
                      PRIVATE protobuf::libprotobuf)
if (lto_supported)
	set_property(TARGET mumble_client_protocol PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

# libmumble_client
find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED)
find_package(Threads REQUIRED)

find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h)
find_library(OPUS_LIBRARY NAMES opus)

add_library(mumble_client
            SHARED
            protocol/control.cpp
            protocol/control.hpp
            protocol/voice.cpp
            protocol/voice.hpp)
set_target_properties(mumble_client PROPERTIES
                      VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
                      SOVERSION 0)
generate_export_header(mumble_client)
target_include_directories(mumble_client
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                           PRIVATE ${OPUS_INCLUDE_DIR})
# enable extra warnings for our code
target_compile_options(mumble_client PRIVATE ${COMPILER_WARNING_OPTIONS})
target_link_libraries(mumble_client
                      PRIVATE mumble_client_protocol
                      PRIVATE Threads::Threads
                      PRIVATE Boost::boost
                      PRIVATE ${OPUS_LIBRARY})
if (lto_supported)
	set_property(TARGET mumble_client PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()