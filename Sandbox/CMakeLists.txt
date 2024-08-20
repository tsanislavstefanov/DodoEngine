cmake_minimum_required(VERSION 3.28.1)
project(Sandbox)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add includes
include_directories(Source)

# Add source files
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/Source/*.cpp")

# Create executable
add_executable(App ${SOURCE_FILES})