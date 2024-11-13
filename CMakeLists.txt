cmake_minimum_required(VERSION 3.27.1)

project(DodoEngine VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(Dodo)
add_subdirectory(ThirdParty)