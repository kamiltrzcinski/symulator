add_library(srk_ebilock STATIC
    src/ebilock_system.cpp
)

target_include_directories(srk_ebilock PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(srk_ebilock PUBLIC engine srk_common)
target_compile_features(srk_ebilock PUBLIC cxx_std_20)
