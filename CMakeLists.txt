cmake_minimum_required(VERSION 3.16)
project(toki)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/get-cpm.cmake)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
pkg_check_modules(GLEW REQUIRED IMPORTED_TARGET glew)
pkg_check_modules(SKIA REQUIRED IMPORTED_TARGET skia)
# TODO: Build Skia via CPM instead of pkg-config, similar to SDL3:
# CPMAddPackage(
#     NAME skia
#     GIT_REPOSITORY "https://skia.googlesource.com/skia.git"
#     GIT_TAG "chrome/m131"  # or pin a specific commit
#     DOWNLOAD_ONLY YES
# )
# Skia uses gn/ninja, so you'd need a custom build step or a CMake wrapper
# like https://github.com/aspect-build/aspect-skia or similar.
CPMAddPackage(
    NAME SDL3
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-3.4.2
    OPTIONS
        "SDL_SHARED OFF"
        "SDL_STATIC ON"
)

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.91.8
    DOWNLOAD_ONLY YES
)

if(imgui_ADDED)
    add_subdirectory(cmake/imgui)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    SDL3::SDL3
    PkgConfig::MPV
    PkgConfig::GLEW
    PkgConfig::SKIA
    imgui
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_include_directories(${PROJECT_NAME} PRIVATE ${SKIA_INCLUDE_DIRS})

# IMGUI Example

add_executable(imgui-example
    src/example.cpp
)

target_link_libraries(imgui-example
    SDL3::SDL3
    PkgConfig::MPV
    PkgConfig::GLEW
    imgui
)

target_compile_features(imgui-example PRIVATE cxx_std_23)