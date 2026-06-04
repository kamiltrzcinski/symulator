cmake_minimum_required(VERSION 3.25)

add_executable(tests_scenario_validation
    test_layer1.cpp
    test_layer2.cpp
)

target_link_libraries(tests_scenario_validation
    PRIVATE
        scenario_validation
        GTest::gtest_main
)

target_compile_features(tests_scenario_validation PRIVATE cxx_std_20)

# Scenario directory for happy-path tests.
target_compile_definitions(tests_scenario_validation PRIVATE
    SCENARIO_DIR="${CMAKE_SOURCE_DIR}/scenarios/gdynia_orlowo"
)

symulator_discover_gtest(tests_scenario_validation
    GROUP_LABEL group:unit-validation
)
