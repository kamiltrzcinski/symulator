qt_add_executable(symulator-client
    src/main.cpp
    include/audio/audio_service.hpp
    src/audio/audio_service.cpp
    resources/audio.qrc
    include/thales/thales_window.hpp
    src/thales/thales_window.cpp
    include/thales/thales_browser.hpp
    src/thales/thales_browser.cpp
    include/thales/topology_loader.hpp
    src/thales/topology_loader.cpp
    include/thales/graphics/thales_element_graphic.hpp
    src/thales/graphics/thales_element_graphic.cpp
    include/thales/graphics/thales_button_item.hpp
    src/thales/graphics/thales_button_item.cpp
    include/thales/graphics/thales_command_box_item.hpp
    src/thales/graphics/thales_command_box_item.cpp
    include/thales/graphics/thales_derail_item.hpp
    src/thales/graphics/thales_derail_item.cpp
    include/thales/graphics/thales_end_catena_item.hpp
    src/thales/graphics/thales_end_catena_item.cpp
    include/thales/graphics/thales_header_item.hpp
    src/thales/graphics/thales_header_item.cpp
    include/thales/graphics/thales_label_item.hpp
    src/thales/graphics/thales_label_item.cpp
    include/thales/graphics/thales_line_block_item.hpp
    src/thales/graphics/thales_line_block_item.cpp
    include/thales/graphics/thales_pkpm_item.hpp
    src/thales/graphics/thales_pkpm_item.cpp
    include/thales/graphics/thales_platform_item.hpp
    src/thales/graphics/thales_platform_item.cpp
    include/thales/graphics/thales_signal_box_item.hpp
    src/thales/graphics/thales_signal_box_item.cpp
    include/thales/graphics/thales_signal_item.hpp
    src/thales/graphics/thales_signal_item.cpp
    include/thales/graphics/thales_switch_item.hpp
    src/thales/graphics/thales_switch_item.cpp
    include/thales/graphics/thales_track_item.hpp
    src/thales/graphics/thales_track_item.cpp
)

target_include_directories(symulator-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/audio
)

target_link_libraries(symulator-client PRIVATE
    engine
    trackview
    symulator_proto
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    flatbuffers::flatbuffers
)

target_compile_features(symulator-client PRIVATE cxx_std_20)

symulator_deploy_qt_conf(symulator-client)
