qt_add_executable(symulator-client
    src/main.cpp
)

target_include_directories(symulator-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(symulator-client PRIVATE
    trackview
    symulator_proto
    Qt6::Widgets
    Qt6::Network
    flatbuffers::flatbuffers
)

target_compile_features(symulator-client PRIVATE cxx_std_20)
