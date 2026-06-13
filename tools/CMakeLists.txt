option(BUILD_TOOLS "Build Qt developer tools" OFF)

if(NOT BUILD_TOOLS)
    return()
endif()

add_subdirectory(shared)
add_subdirectory(uid-generator)
add_subdirectory(vehicle-browser)
