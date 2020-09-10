enable_testing()

find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS unit_test_framework)

add_executable(test_libmumble_client_protocol protocol/main.cpp protocol/voice.cpp)
target_include_directories(test_libmumble_client_protocol
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE mumble_client_protocol
                           PRIVATE ${Boost_INCLUDE_DIR})
target_compile_definitions(test_libmumble_client_protocol PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(test_libmumble_client_protocol PUBLIC mumble_client_protocol PRIVATE Boost::unit_test_framework)
add_test(NAME ProtocolUnitTest COMMAND test_libmumble_client_protocol)