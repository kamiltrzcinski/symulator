add_library(srk_common STATIC
    src/device_rules.cpp
    src/route_graph.cpp
)

target_include_directories(srk_common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(srk_common PUBLIC engine)
target_compile_features(srk_common PUBLIC cxx_std_20)
