qt_add_executable(uid-generator
    main.cpp
    ui/main_window.cpp
    ui/uid_form.cpp
    ui/uid_registry_view.cpp
    ui/uid_result_view.cpp
)

target_include_directories(uid-generator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(uid-generator PRIVATE
    tools_shared
    Qt6::Widgets
)

target_compile_features(uid-generator PRIVATE cxx_std_20)

symulator_deploy_qt_conf(uid-generator)
