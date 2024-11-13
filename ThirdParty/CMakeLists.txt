cmake_minimum_required(VERSION 3.27.1)

# spdlog:
set(SPDLOG_BUILD_SHARED OFF)
add_subdirectory(spdlog)

# yaml-cpp:
add_subdirectory(yaml-cpp)