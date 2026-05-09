add_subdirectory(engine)
add_subdirectory(server)
add_subdirectory(integration)
add_subdirectory(proto)

if(TARGET Qt6::Core)
    add_subdirectory(qt6)
endif()
