# mumble_packet
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(
        MUMBLE_PROTOBUF_SOURCE_FILES
        MUMBLE_PROTOBUF_HEADER_FILES
        Mumble.proto
)

add_library(
        mumble_protocol
        STATIC
        ${MUMBLE_PROTOBUF_SOURCE_FILES}
        ${MUMBLE_PROTOBUF_HEADER_FILES}
)

target_include_directories(
        mumble_protocol
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(
        mumble_protocol
        PUBLIC Threads::Threads
        PUBLIC protobuf::libprotobuf
)
