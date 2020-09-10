include(GenerateExportHeader)
# check for LTO support in the compiler
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED)

# libmumble_client_protocol
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(MUMBLE_PROTOBUF_SOURCE_FILES MUMBLE_PROTOBUF_HEADER_FILES protocol/Mumble.proto)

add_library(mumble_client_protocol
            SHARED
            protocol/control.cpp
            protocol/control.hpp
            protocol/voice.cpp
            protocol/voice.hpp
            ${MUMBLE_PROTOBUF_SOURCE_FILES})
generate_export_header(mumble_client_protocol)
target_include_directories(mumble_client_protocol
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                           PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(mumble_client_protocol PRIVATE protobuf::libprotobuf)

# enable extra warnings for our code
if (${MSVC})
	target_compile_options(mumble_client_protocol PUBLIC /W4 /EHsc)
else ()
	target_compile_options(mumble_client_protocol PUBLIC -Wall -Wextra -Wpedantic)
endif ()

if (lto_supported)
	set_property(TARGET mumble_client_protocol PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()