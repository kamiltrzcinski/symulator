add_library(trackview STATIC
    src/placeholder.cpp
)

target_include_directories(trackview PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(trackview PUBLIC
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

target_compile_features(trackview PUBLIC cxx_std_20)
