# mumble_packet
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

add_library(
        mumble_protocol_common
        STATIC
        src/Mumble.proto
        src/MumbleUDP.proto
        src/packet.cpp
        src/packet.hpp
        src/pimpl.hpp
        src/pimpl_impl.hpp
        src/util.cpp
        src/util.hpp
)
protobuf_generate(
    TARGET mumble_protocol_common
)
generate_export_header(mumble_protocol_common)

set_target_properties(
        mumble_protocol_common
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_WARNING_AS_ERROR ON
        POSITION_INDEPENDENT_CODE ON
        INTERPROCEDURAL_OPTIMIZATION ${LTO_SUPPORTED}
)

target_include_directories(
        mumble_protocol_common
        PUBLIC src
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src
)

target_link_libraries(
        mumble_protocol_common
        PUBLIC protobuf::libprotobuf
        PUBLIC Threads::Threads
)

if (${BUILD_TEST})
    # These tests can use the Catch2-provided main
    add_executable(
            mumble_protocol_common_test
            test/util.cpp
    )

    set_target_properties(
            mumble_protocol_common_test
            PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    target_link_libraries(
            mumble_protocol_common_test
            PRIVATE mumble_protocol_common
            PRIVATE Catch2::Catch2WithMain
    )
    catch_discover_tests(mumble_protocol_common_test)
endif ()
