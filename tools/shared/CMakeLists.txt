add_library(tools_shared STATIC
    data/directory_data_source.cpp
    data/json_loader.cpp
    data/packages_data_source.cpp
    registry/uid_registry.cpp
    registry/uid_validator.cpp
    services/clipboard_service.cpp
    services/uid_generator_service.cpp
    ui/uid_legend_panel.cpp
)

target_include_directories(tools_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../engine/include>
)

target_link_libraries(tools_shared PUBLIC
    nlohmann_json::nlohmann_json
    Qt6::Widgets
)

target_compile_features(tools_shared PUBLIC cxx_std_20)

add_subdirectory(tests)
