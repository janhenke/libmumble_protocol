add_executable(
        mumble_server
        src/main.cpp
)
if (WIN32)
    target_sources(
            mumble_server
            PUBLIC
            src/application.manifest
    )
endif ()

set_target_properties(
        mumble_server
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options)
find_package(libpqxx CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(
        mumble_server
        PRIVATE mumble_protocol
        PRIVATE Boost::boost
        PRIVATE Boost::program_options
        PRIVATE libpqxx::pqxx
        PRIVATE spdlog::spdlog
)
