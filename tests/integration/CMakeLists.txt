add_executable(tests_integration
    test_placeholder.cpp
    test_pg_db_writer.cpp
)
target_link_libraries(tests_integration PRIVATE server_db_lib GTest::gtest_main)
target_compile_features(tests_integration PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests_integration
    PROPERTIES LABELS "integration"
)
