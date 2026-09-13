set(CLIENT_SOURCES
    src/main.cpp
    include/thales/thales_window.hpp
    src/thales/thales_window.cpp
    include/thales/thales_browser.hpp
    src/thales/thales_browser.cpp
    include/thales/topology_loader.hpp
    src/thales/topology_loader.cpp
    resources/thales.qrc
)

if(TARGET Qt6::Multimedia)
    list(APPEND CLIENT_SOURCES
        include/audio/audio_service.hpp
        src/audio/audio_service.cpp
        resources/audio.qrc
    )
endif()

qt_add_executable(symulator-client
    ${CLIENT_SOURCES}
)
set_target_properties(symulator-client PROPERTIES AUTORCC ON)

target_include_directories(symulator-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/audio
)

target_link_libraries(symulator-client PRIVATE
    trackview
    symulator_proto
    Qt6::Widgets
    Qt6::Network
    flatbuffers::flatbuffers
)

if(TARGET Qt6::Multimedia)
    target_link_libraries(symulator-client PRIVATE Qt6::Multimedia)
endif()

target_compile_features(symulator-client PRIVATE cxx_std_20)

symulator_deploy_qt_conf(symulator-client)

