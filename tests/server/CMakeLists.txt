add_executable(tests_server_protocol
    test_placeholder.cpp
    test_frame.cpp
    test_command_ingress.cpp
    test_ownership_guard.cpp
)
target_link_libraries(tests_server_protocol PRIVATE server_lib tests_common GTest::gtest_main)
target_compile_features(tests_server_protocol PRIVATE cxx_std_20)

symulator_discover_gtest(tests_server_protocol
    GROUP_LABEL group:unit-server-protocol
)

add_executable(tests_server_dispatch
    test_dispatch_bus.cpp
    test_dispatch_exchange_manager.cpp
    test_dispatch_coordinator.cpp
    test_dispatch_channel.cpp
    test_pip_writer.cpp
)
target_link_libraries(tests_server_dispatch PRIVATE server_lib tests_common GTest::gtest_main)
target_compile_features(tests_server_dispatch PRIVATE cxx_std_20)

symulator_discover_gtest(tests_server_dispatch
    GROUP_LABEL group:unit-server-dispatch
)

add_executable(tests_server_terminal
    test_terminal_session.cpp
    test_terminal_commands.cpp
)
target_link_libraries(tests_server_terminal PRIVATE server_lib tests_common GTest::gtest_main)
target_compile_features(tests_server_terminal PRIVATE cxx_std_20)

symulator_discover_gtest(tests_server_terminal
    GROUP_LABEL group:unit-server-terminal
)
