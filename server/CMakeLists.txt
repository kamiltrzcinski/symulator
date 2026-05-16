# Build a static library of server components so tests can link against them
# without also pulling in main().
add_library(server_lib STATIC
    src/frame.cpp
    src/command_ingress.cpp
    src/ownership_guard.cpp
    src/transport_gateway.cpp
    src/dispatch_bus.cpp
)

target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(server_lib PUBLIC
    engine
    symulator_proto
    flatbuffers::flatbuffers
    OpenSSL::SSL
    OpenSSL::Crypto
    asio::asio
)

target_compile_features(server_lib PUBLIC cxx_std_20)

# Main server executable
add_executable(symulator-server
    src/main.cpp
)

target_link_libraries(symulator-server PRIVATE server_lib)
target_compile_features(symulator-server PRIVATE cxx_std_20)
