find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

add_library(
        mumble_protocol
        STATIC
        src/Mumble.proto
        src/MumbleUDP.proto
        src/mumble_protocol.cppm
        src/mumble_protocol_client.cppm
        src/mumble_protocol_packet.cppm
        src/mumble_protocol_server.cppm
        src/mumble_protocol_utility.cppm
)
protobuf_generate(
        TARGET mumble_protocol
)

# Disable specific compiler warnings for generated file (because we cannot fix the warnings)
set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/src/Mumble.pb.cc
        ${CMAKE_CURRENT_BINARY_DIR}/src/MumbleUDP.pb.cc
        PROPERTIES
        COMPILE_FLAGS $<IF:$<CXX_COMPILER_ID:MSVC>,/wd4244,-Wno-error=narrowing>
)

set_target_properties(
        mumble_protocol
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
        INTERPROCEDURAL_OPTIMIZATION ${LTO_SUPPORTED}
)

if (WIN32)
    target_compile_definitions(
            mumble_protocol
            PUBLIC
            _WIN32_WINNT=0x0A00
    )
endif ()

target_include_directories(
        mumble_protocol
        PUBLIC src
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src
)

target_link_libraries(
        mumble_protocol
        PUBLIC protobuf::libprotobuf
        PUBLIC Threads::Threads
)

if (${BUILD_TEST})
    # These tests can use the Catch2-provided main
    add_executable(
            mumble_protocol_test
            test/util.cpp
    )

    set_target_properties(
            mumble_protocol_test
            PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    target_link_libraries(
            mumble_protocol_test
            PRIVATE mumble_protocol
            PRIVATE Catch2::Catch2WithMain
    )
    catch_discover_tests(mumble_protocol_test)
endif ()
