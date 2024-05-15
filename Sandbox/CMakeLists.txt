cmake_minimum_required(VERSION 3.28.1)
project(Sandbox)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add includes
include_directories(Source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Core/Source)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

# Add source files
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/Source/*.cpp")

# Create executable
add_executable(Sandbox ${SOURCE_FILES})

# Link precompiled header
target_precompile_headers(Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../Core/Source/pch.h)

# Link libraries
target_link_libraries(Sandbox CoreEngine)
target_link_libraries(Sandbox ${Vulkan_LIBRARIES})

# Add platform-specific definitions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Sandbox PRIVATE _WIN32)
endif()

# Set debug and release symbols
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")