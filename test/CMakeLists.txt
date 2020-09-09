find_package(Boost 1.71 REQUIRED COMPONENTS unit_test_framework)

add_executable(protocol_test protocol/main.cpp protocol/voice.cpp)

target_include_directories(protocol_test PRIVATE ${Boost_INCLUDE_DIR})
target_compile_definitions(protocol_test PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(protocol_test PUBLIC mumble_client_protocol PRIVATE Boost::unit_test_framework)
add_test(NAME ProtocolUnitTest COMMAND protocol_test)