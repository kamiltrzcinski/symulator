set(ENGINE_TEST_SCENARIO_DIR "${CMAKE_SOURCE_DIR}/scenarios/gdynia_orlowo")

function(symulator_configure_engine_unit_target target)
    target_link_libraries(${target} PRIVATE engine tests_common GTest::gtest_main)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_compile_definitions(${target} PRIVATE
        SCENARIO_DIR="${ENGINE_TEST_SCENARIO_DIR}"
    )
endfunction()

add_executable(tests_engine_foundation
    test_placeholder.cpp
    test_uid_codec.cpp
    test_types.cpp
)
symulator_configure_engine_unit_target(tests_engine_foundation)
symulator_discover_gtest(tests_engine_foundation
    GROUP_LABEL group:unit-engine-foundation
)

add_executable(tests_engine_concurrency
    test_event_queue.cpp
    test_priority_command_queue.cpp
    test_event_dispatcher.cpp
)
symulator_configure_engine_unit_target(tests_engine_concurrency)
symulator_discover_gtest(tests_engine_concurrency
    GROUP_LABEL group:unit-engine-concurrency
)

add_executable(tests_engine_state
    test_state_applier.cpp
    test_topology_loader.cpp
)
symulator_configure_engine_unit_target(tests_engine_state)
symulator_discover_gtest(tests_engine_state
    GROUP_LABEL group:unit-engine-state
)

add_executable(tests_engine_snapshot
    test_snapshot_service.cpp
)
symulator_configure_engine_unit_target(tests_engine_snapshot)
symulator_discover_gtest(tests_engine_snapshot
    GROUP_LABEL group:unit-engine-snapshot
)

add_executable(tests_engine_fleet
    test_fleet_registry_loading.cpp
    test_fleet_registry_coupling.cpp
    test_fleet_registry_carriers.cpp
)
symulator_configure_engine_unit_target(tests_engine_fleet)
symulator_discover_gtest(tests_engine_fleet
    GROUP_LABEL group:unit-engine-fleet
)

add_executable(tests_engine_simulation
    test_physics.cpp
    test_train_sim.cpp
    test_train_fleet.cpp
    test_engine_loop.cpp
)
symulator_configure_engine_unit_target(tests_engine_simulation)
symulator_discover_gtest(tests_engine_simulation
    GROUP_LABEL group:unit-engine-simulation
)
