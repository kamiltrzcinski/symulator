add_executable(tests_server
    test_placeholder.cpp
    test_frame.cpp
    test_command_ingress.cpp
    test_ownership_guard.cpp
    test_dispatch_bus.cpp
    test_dispatch_exchange_manager.cpp
    test_bilateral_channel.cpp
    test_pip_writer.cpp
)
target_link_libraries(tests_server PRIVATE server_lib GTest::gtest_main)
target_compile_features(tests_server PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests_server)
