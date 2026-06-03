add_executable(tests_integration
    test_placeholder.cpp
    test_pg_db_writer.cpp
    test_edr_coordinator.cpp
    test_pip_track_state.cpp
    test_uid_registry.cpp
)
target_link_libraries(tests_integration PRIVATE server_db_lib engine GTest::gtest_main)
target_compile_features(tests_integration PRIVATE cxx_std_20)
target_compile_definitions(tests_integration PRIVATE REPO_ROOT="${CMAKE_SOURCE_DIR}")

include(GoogleTest)
gtest_discover_tests(tests_integration
    PROPERTIES LABELS "integration"
)
