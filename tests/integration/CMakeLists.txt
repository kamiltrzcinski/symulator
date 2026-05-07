add_executable(tests_integration
    test_placeholder.cpp
)
target_link_libraries(tests_integration PRIVATE engine GTest::gtest_main)
target_compile_features(tests_integration PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests_integration)
