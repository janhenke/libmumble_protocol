find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS program_options)

add_executable(libmumble_client_example main.cpp)
target_include_directories(libmumble_client_example
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE mumble_client_protocol
                           PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(libmumble_client_example PRIVATE mumble_client_protocol PRIVATE Boost::program_options)