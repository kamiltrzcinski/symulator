function(symulator_discover_gtest target)
    set(options)
    set(oneValueArgs GROUP_LABEL)
    set(multiValueArgs)
    cmake_parse_arguments(SYM "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT SYM_GROUP_LABEL)
        message(FATAL_ERROR "symulator_discover_gtest(${target}) requires GROUP_LABEL")
    endif()

    include(GoogleTest)
    gtest_discover_tests(${target}
        PROPERTIES LABELS "${SYM_GROUP_LABEL}"
    )
endfunction()

add_subdirectory(engine)
add_subdirectory(common)
add_subdirectory(srk)
add_subdirectory(server)
add_subdirectory(integration)
add_subdirectory(proto)
add_subdirectory(libscenario_validation)

if(TARGET trackview_core)
    add_subdirectory(libtrackview)
endif()

if(TARGET Qt6::Core)
    add_subdirectory(qt6)
endif()
