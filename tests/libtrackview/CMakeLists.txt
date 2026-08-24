add_executable(tests_libtrackview
    main.cpp
    test_attachment_resolver.cpp
    test_engine_adapters.cpp
    test_json_layout_reader.cpp
    test_layout_validator.cpp
    test_qt_rendering.cpp
    test_render_model_builder.cpp
    test_sopot_integration.cpp
)

target_link_libraries(tests_libtrackview PRIVATE
    trackview
    trackview_engine_adapter
    engine
    GTest::gtest
    Qt6::Widgets
)

target_compile_definitions(tests_libtrackview PRIVATE
    TRACKVIEW_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    SOPOT_SCENARIO_DIR="${CMAKE_SOURCE_DIR}/scenarios/sopot"
)

if(MSVC)
    target_compile_options(tests_libtrackview PRIVATE /EHsc)
endif()

add_test(NAME tests_libtrackview COMMAND tests_libtrackview)
set_tests_properties(tests_libtrackview PROPERTIES
    LABELS "group:unit-libtrackview"
)
symulator_set_qt_test_environment(tests_libtrackview)
