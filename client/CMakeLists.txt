add_executable(
        mumble_client
        src/main.cpp
)
if (WIN32)
    target_sources(
            mumble_client
            PUBLIC
            src/application.manifest
    )
endif ()

set_target_properties(
        mumble_client
        PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options)
find_package(OpenSSL CONFIG REQUIRED)
pkg_check_modules(Opus REQUIRED IMPORTED_TARGET opus)
find_package(RtAudio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(
        mumble_client
        PRIVATE mumble_protocol
        PRIVATE Boost::boost
        PRIVATE Boost::program_options
        PRIVATE OpenSSL::SSL
        PRIVATE OpenSSL::Crypto
        PRIVATE PkgConfig::Opus
        PRIVATE RtAudio::rtaudio
        PRIVATE spdlog::spdlog
)
