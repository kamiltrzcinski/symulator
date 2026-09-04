qt_add_executable(symulator-client
    src/main.cpp
    include/audio/audio_service.hpp
    src/audio/audio_service.cpp
    resources/audio.qrc
)

target_include_directories(symulator-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(symulator-client PRIVATE
    trackview
    symulator_proto
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    flatbuffers::flatbuffers
)

target_compile_features(symulator-client PRIVATE cxx_std_20)

symulator_deploy_qt_conf(symulator-client)
