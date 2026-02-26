cmake_minimum_required(VERSION 3.16)
project(toolkit)

add_library(${PROJECT_NAME} STATIC
    src/app.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
