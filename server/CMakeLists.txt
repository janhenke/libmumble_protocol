
add_library(
        mumble_server
        SHARED
        lib/server.cpp
        lib/server.hpp
)
generate_export_header(mumble_server)

set_target_properties(
        mumble_server
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
        mumble_server
        PUBLIC lib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(
        mumble_server
        PUBLIC ASIO_NO_DEPRECATED
)

find_package(asio CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(
        mumble_server
        PUBLIC mumble_protocol_common
        PRIVATE asio::asio
        PRIVATE OpenSSL::SSL
        PRIVATE OpenSSL::Crypto
        PRIVATE spdlog::spdlog
        PRIVATE Threads::Threads
)

add_executable(
        mumble_server_app
        app/main.cpp
)
if (WIN32)
    target_sources(
            mumble_server_app
            PUBLIC
            app/application.manifest
    )
endif ()

set_target_properties(
        mumble_server_app
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options)
pkg_check_modules(libpqxx REQUIRED IMPORTED_TARGET libpqxx)

target_link_libraries(
        mumble_server_app
        PRIVATE PkgConfig::libpqxx
        PRIVATE mumble_server
        PRIVATE Boost::boost
        PRIVATE Boost::program_options
        PRIVATE spdlog::spdlog
)
