add_library(trackview_core STATIC
    src/json_layout_reader.cpp
    src/layout_validator.cpp
    src/path_attachment_resolver.cpp
    src/render_model_builder.cpp
)

target_include_directories(trackview_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(trackview_core PRIVATE
    nlohmann_json::nlohmann_json
)

target_compile_features(trackview_core PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(trackview_core PRIVATE /EHsc)
endif()

add_library(trackview_engine_adapter STATIC
    src/engine_track_state_adapter.cpp
)

target_link_libraries(trackview_engine_adapter PUBLIC
    trackview_core
    engine
)

target_compile_features(trackview_engine_adapter PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(trackview_engine_adapter PRIVATE /EHsc)
endif()

add_library(trackview STATIC
    src/ebiscreen_theme.cpp
    src/qt_scene_renderer.cpp
)

target_link_libraries(trackview PUBLIC
    trackview_core
    Qt6::Widgets
)

target_compile_features(trackview PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(trackview PRIVATE /EHsc)
endif()
