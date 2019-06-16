find_package(Protobuf)

protobuf_generate_cpp(MUMBLE_PROTOBUF_SOURCE_FILES MUMBLE_PROTOBUF_HEADER_FILES Mumble.proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(mumble_protocol
            SHARED
            ${MUMBLE_PROTOBUF_SOURCE_FILES}
            ${MUMBLE_PROTOBUF_HEADER_FILES})

# enable extra warnings for our code
if (${MSVC})
	target_compile_options(mumble_protocol PUBLIC /W4 /EHsc)
else ()
	target_compile_options(mumble_protocol PUBLIC -Wall -Wextra -Wpedantic)
endif ()

target_include_directories(mumble_protocol
                           PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(mumble_protocol
                      PRIVATE protobuf::libprotobuf)
