add_library(vehicle_browser_models STATIC
    models/train_model.cpp
    models/vehicle_model.cpp
    models/vehicle_type_model.cpp
)

target_include_directories(vehicle_browser_models PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vehicle_browser_models PUBLIC
    tools_shared
    Qt6::Widgets
)

target_compile_features(vehicle_browser_models PUBLIC cxx_std_20)

qt_add_executable(vehicle-browser
    main.cpp
    application/browser_data_controller.cpp
    ui/main_window.cpp
    ui/train_builder_panel.cpp
    ui/vehicle_edit_dialog.cpp
    ui/vehicle_panel.cpp
    ui/vehicle_type_panel.cpp
)

target_include_directories(vehicle-browser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vehicle-browser PRIVATE
    vehicle_browser_models
    tools_shared
    Qt6::Widgets
)

target_compile_features(vehicle-browser PRIVATE cxx_std_20)

add_subdirectory(tests)
