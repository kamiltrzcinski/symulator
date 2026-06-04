add_library(tests_common INTERFACE)

target_include_directories(tests_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tests_common INTERFACE cxx_std_20)
