function(add_vehicle_browser_test target source)
    qt_add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE vehicle_browser_models tools_shared Qt6::Test)
    target_compile_features(${target} PRIVATE cxx_std_20)
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
        LABELS "tools"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endfunction()

add_vehicle_browser_test(tools_test_vehicle_type_model test_vehicle_type_model.cpp)
add_vehicle_browser_test(tools_test_vehicle_model test_vehicle_model.cpp)
add_vehicle_browser_test(tools_test_train_model test_train_model.cpp)

qt_add_executable(tools_test_editor_persistence
    test_editor_persistence.cpp
    ../application/editor_persistence_service.cpp
    ../ui/train_builder_panel.cpp
    ../ui/vehicle_edit_dialog.cpp
)
target_link_libraries(tools_test_editor_persistence PRIVATE
    vehicle_browser_models
    tools_shared
    Qt6::Test
    Qt6::Widgets
)
target_compile_features(tools_test_editor_persistence PRIVATE cxx_std_20)
add_test(NAME tools_test_editor_persistence COMMAND tools_test_editor_persistence)
set_tests_properties(tools_test_editor_persistence PROPERTIES
    LABELS "tools"
    ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
)
