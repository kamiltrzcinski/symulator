add_executable(tests_engine
    test_placeholder.cpp
    test_event_queue.cpp
    test_priority_command_queue.cpp
    test_event_dispatcher.cpp
    test_types.cpp
    test_physics.cpp
    test_fleet_registry.cpp
    test_train_sim.cpp
    test_state_applier.cpp
    test_topology_loader.cpp
    test_engine_loop.cpp
    test_snapshot_service.cpp
)
target_link_libraries(tests_engine PRIVATE engine GTest::gtest_main)
target_compile_features(tests_engine PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests_engine)
