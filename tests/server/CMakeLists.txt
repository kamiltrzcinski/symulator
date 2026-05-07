add_executable(tests_server
    test_placeholder.cpp
)
target_link_libraries(tests_server PRIVATE engine GTest::gtest_main)
target_compile_features(tests_server PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests_server)
