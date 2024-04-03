
add_library(
        mumble_client
        SHARED
        lib/client.cpp
        lib/client.hpp
)
generate_export_header(mumble_client)

set_target_properties(
        mumble_client
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
        INTERPROCEDURAL_OPTIMIZATION ${LTO_SUPPORTED}
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        SOVERSION 0
)

target_include_directories(
        mumble_client
        PUBLIC lib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(
        mumble_client
        PUBLIC ASIO_NO_DEPRECATED
)

# enable extra warnings for our code
target_compile_options(
        mumble_client
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

find_package(asio CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(
        mumble_client
        PUBLIC mumble_protocol_common
        PRIVATE asio::asio
        PRIVATE OpenSSL::SSL
        PRIVATE OpenSSL::Crypto
        PRIVATE spdlog::spdlog
        PRIVATE Threads::Threads
)

add_executable(
        mumble_client_app
        app/main.cpp
)
if (WIN32)
    target_sources(
            mumble_client_app
            PUBLIC
            app/application.manifest
    )
endif ()

set_target_properties(
        mumble_client_app
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options)
find_package(Opus CONFIG REQUIRED)
find_package(RtAudio CONFIG REQUIRED)
target_link_libraries(
        mumble_client_app
        PRIVATE mumble_client
        PRIVATE Boost::boost
        PRIVATE Boost::program_options
        PRIVATE Opus::opus
        PRIVATE RtAudio::rtaudio
        PRIVATE spdlog::spdlog
)
