add_executable(tests_srk
    test_ebilock_interlocking.cpp
    test_ml8_shl12.cpp
    test_srk_common_route_graph.cpp
    test_srk_common_device_rules.cpp
)

target_link_libraries(tests_srk PRIVATE
    srk_ebilock
    srk_ml8
    srk_common
    engine
    GTest::gtest_main
)
target_compile_features(tests_srk PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tests_srk)
