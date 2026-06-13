function(add_tools_shared_test target source)
    qt_add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE tools_shared Qt6::Test)
    target_compile_features(${target} PRIVATE cxx_std_20)
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES
        LABELS "tools"
        ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endfunction()

add_tools_shared_test(tools_test_uid_registry test_uid_registry.cpp)
add_tools_shared_test(tools_test_uid_validator test_uid_validator.cpp)
add_tools_shared_test(tools_test_uid_generator_service test_uid_generator_service.cpp)
add_tools_shared_test(tools_test_json_loader test_json_loader.cpp)
