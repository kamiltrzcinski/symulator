add_executable(tests_integration_non_db
    test_placeholder.cpp
    test_uid_registry.cpp
    test_world_loading.cpp
    test_unknown_uid_fallback.cpp
    test_cross_section_transit.cpp
    test_spawn_despawn_transit.cpp
)
target_link_libraries(tests_integration_non_db PRIVATE engine GTest::gtest_main)
target_compile_features(tests_integration_non_db PRIVATE cxx_std_20)
target_compile_definitions(tests_integration_non_db PRIVATE REPO_ROOT="${CMAKE_SOURCE_DIR}")

symulator_discover_gtest(tests_integration_non_db
    GROUP_LABEL group:integration-non-db
)

add_executable(tests_integration_db
    test_pg_db_writer.cpp
    test_edr_coordinator.cpp
    test_pip_track_state.cpp
)
target_link_libraries(tests_integration_db PRIVATE
    tests_common
    server_db_lib
    engine
    GTest::gtest_main
)
target_compile_features(tests_integration_db PRIVATE cxx_std_20)

symulator_discover_gtest(tests_integration_db
    GROUP_LABEL group:integration-db
)
