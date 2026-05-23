# Build a static library of server components so tests can link against them
# without also pulling in main().
add_library(server_lib STATIC
    src/frame.cpp
    src/command_ingress.cpp
    src/ownership_guard.cpp
    src/transport_gateway.cpp
    src/dispatch_bus.cpp
    src/dispatch_exchange_manager.cpp
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

# Main server executable.
# session_server.cpp is the composition root — it lives here, not in server_lib,
# so that unit tests do not pull in the full startup wiring.
add_executable(symulator-server
    src/main.cpp
    src/session_server.cpp
)

# Link srk_ebilock and srk_ml8 with --whole-archive so their static-init
# self-registration in ControlSystemRegistry is executed before main().
# Without WHOLE_ARCHIVE the linker would drop the unreferenced object files.
target_link_libraries(symulator-server PRIVATE
    server_lib
    $<LINK_LIBRARY:WHOLE_ARCHIVE,srk_ebilock,srk_ml8>
)
target_compile_features(symulator-server PRIVATE cxx_std_20)
