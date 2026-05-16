add_library(srk_ml8 STATIC
    src/ml8_system.cpp
)

target_include_directories(srk_ml8 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(srk_ml8 PUBLIC engine srk_common)
target_compile_features(srk_ml8 PUBLIC cxx_std_20)
