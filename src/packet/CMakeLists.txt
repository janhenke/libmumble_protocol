# mumble_packet
find_package(Protobuf REQUIRED)
protobuf_generate_cpp(MUMBLE_PROTOBUF_SOURCE_FILES MUMBLE_PROTOBUF_HEADER_FILES Mumble.proto)
add_library(mumble_packet
            SHARED
            ${MUMBLE_PROTOBUF_SOURCE_FILES} ${MUMBLE_PROTOBUF_HEADER_FILES}
            tcp_packet.cpp tcp_packet.hpp)
set_target_properties(mumble_packet PROPERTIES
                      VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
                      SOVERSION 0
                      WINDOWS_EXPORT_ALL_SYMBOLS ON)
generate_export_header(mumble_packet)
target_include_directories(mumble_packet
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
                           PRIVATE ${Protobuf_INCLUDE_DIRS})
target_link_libraries(mumble_packet
                      PRIVATE ${CMAKE_THREAD_LIBS_INIT}
                      PRIVATE protobuf::libprotobuf)
if (lto_supported)
	set_property(TARGET mumble_packet PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()