add_executable(tests_proto
    test_generated_headers.cpp
)

target_link_libraries(tests_proto PRIVATE
    symulator_proto
    GTest::gtest_main
)

target_compile_features(tests_proto PRIVATE cxx_std_20)
add_dependencies(tests_proto generate_proto_headers)

include(GoogleTest)
gtest_discover_tests(tests_proto)
