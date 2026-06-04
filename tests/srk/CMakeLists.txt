function(symulator_configure_srk_unit_target target)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target} PRIVATE
        srk_common
        engine
        tests_common
        GTest::gtest_main
    )
endfunction()

add_executable(tests_srk_ebilock
    test_ebilock_interlocking.cpp
)
target_link_libraries(tests_srk_ebilock PRIVATE srk_ebilock)
symulator_configure_srk_unit_target(tests_srk_ebilock)
symulator_discover_gtest(tests_srk_ebilock
    GROUP_LABEL group:unit-srk-ebilock
)

add_executable(tests_srk_ml8
    test_ml8_shl12.cpp
)
target_link_libraries(tests_srk_ml8 PRIVATE srk_ml8)
symulator_configure_srk_unit_target(tests_srk_ml8)
symulator_discover_gtest(tests_srk_ml8
    GROUP_LABEL group:unit-srk-ml8
)

add_executable(tests_srk_common
    test_srk_common_route_graph.cpp
    test_srk_common_device_rules.cpp
)
symulator_configure_srk_unit_target(tests_srk_common)
symulator_discover_gtest(tests_srk_common
    GROUP_LABEL group:unit-srk-common
)
