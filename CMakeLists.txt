cmake_minimum_required(VERSION 3.15)
project(MyProject LANGUAGES CXX)

# Find spdlog via config file
find_package(spdlog REQUIRED CONFIG)

# Add subdirectory for vectormath2d library
add_subdirectory(vectormath2d)
