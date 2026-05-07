qt_add_executable(symulator-editor
    src/main.cpp
)

target_include_directories(symulator-editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(symulator-editor PRIVATE
    trackview
    Qt6::Widgets
    Qt6::Sql
    nlohmann_json::nlohmann_json
)

target_compile_features(symulator-editor PRIVATE cxx_std_20)
