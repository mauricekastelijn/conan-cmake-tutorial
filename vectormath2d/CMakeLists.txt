cmake_minimum_required(VERSION 3.15)

project(vectormath2d LANGUAGES CXX)

add_library(vectormath2d STATIC src/vectormath2d.cpp)

target_include_directories(vectormath2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(vectormath2d PUBLIC spdlog::spdlog)
