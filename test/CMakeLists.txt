find_package(Boost ${BOOST_REQUIRED_VERSION} REQUIRED COMPONENTS unit_test_framework)

add_executable(mumble_client_test
               main.cpp
               protocol/voice.cpp)
target_include_directories(mumble_client_test
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                           PRIVATE mumble_client
                           PRIVATE ${Boost_INCLUDE_DIR})
target_compile_definitions(mumble_client_test PRIVATE BOOST_TEST_DYN_LINK)
target_link_libraries(mumble_client_test PUBLIC mumble_client PRIVATE Boost::unit_test_framework)

add_test(NAME "decode_varint" COMMAND mumble_client_test --catch_system_error=yes --run_test=decode_varint)
add_test(NAME "encode_varint" COMMAND mumble_client_test --catch_system_error=yes --run_test=encode_varint)