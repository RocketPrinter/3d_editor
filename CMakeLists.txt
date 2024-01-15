cmake_minimum_required(VERSION 3.27)
project(3d_editor)

set(CMAKE_CXX_STANDARD 20)

# Dependencies
set(RAYLIB_VERSION 5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

set(NLOHMANN_VERSION 3.11.3)
find_package(nlohmann_json  ${NLOHMANN_VERSION} QUIET) # QUIET or REQUIRED
if (NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            nlohmann_json
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/nlohmann/json/releases/download/v${NLOHMANN_VERSION}/json.tar.xz
    )
    FetchContent_GetProperties(nlohmann_json)
    FetchContent_MakeAvailable(nlohmann_json)
endif()

add_executable(3d_editor src/main.cpp
        src/rendering.cpp
        src/object.cpp
        src/misc.cpp
        src/serialization.cpp
        src/serialization.h
        src/menu.cpp
        src/menu.h
        src/tool.cpp
        src/tool.h
)
#set(raylib_VERBOSE 1)
target_link_libraries(3d_editor raylib nlohmann_json::nlohmann_json)