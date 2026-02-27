cmake_minimum_required(VERSION 3.16)
project(toolkit)

add_library(${PROJECT_NAME} STATIC
    src/app.cpp
    src/window.cpp
    src/ui.cpp
    src/render/renderer.cpp
    src/screen.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC
    core
    spdlog
    yogacore
    PkgConfig::SKIA
    SDL3::SDL3
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
