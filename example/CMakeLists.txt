find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options)

add_executable(
        mumble_client_example
        main.cpp
)

target_include_directories(
        mumble_client_example
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE mumble_client
        PRIVATE ${Boost_INCLUDE_DIR}
        PRIVATE ${SOUDIO_INCLUDE_DIR}
)

find_package(libsoundio CONFIG REQUIRED)

target_link_libraries(
        mumble_client_example
        PRIVATE mumble_client
        PRIVATE Boost::boost
        PRIVATE Boost::program_options
        PRIVATE libsoundio::libsoundio
)
