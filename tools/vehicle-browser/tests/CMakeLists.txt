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
