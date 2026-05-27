cmake_minimum_required(VERSION 3.25)

# ── libscenario_validation ────────────────────────────────────────────────────

find_package(nlohmann_json CONFIG REQUIRED)

add_library(scenario_validation STATIC
    src/validation_result.cpp
    src/layer1_validator.cpp
    src/layer2_validator.cpp
    src/layer3_validator.cpp
)

target_include_directories(scenario_validation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(scenario_validation PUBLIC nlohmann_json::nlohmann_json)
target_compile_features(scenario_validation PUBLIC cxx_std_20)
