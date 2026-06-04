add_executable(tests_qt6
    test_qt6_sanity.cpp
)

target_link_libraries(tests_qt6 PRIVATE
    Qt6::Core
    GTest::gtest_main
)

target_compile_features(tests_qt6 PRIVATE cxx_std_20)

symulator_discover_gtest(tests_qt6
    GROUP_LABEL group:unit-qt6
)
