cmake_minimum_required(VERSION 3.27.1)
project(DodoEngine VERSION 1.0)

# Set C++ standard & export compile commands.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Get all header & source files.
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

# Exclude precompiled header files from sources
list(FILTER SOURCES EXCLUDE REGEX ".*pch\\.(cpp|h)$")

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set(THIRD_PARTY_DIR ${PARENT_DIR}/third_party)
file(GLOB_RECURSE THIRD_PARTY_SOURCES ${THIRD_PARTY_DIR}/*.cpp ${THIRD_PARTY_DIR}/*.h)
list(FILTER THIRD_PARTY_SOURCES EXCLUDE REGEX ".*pch\\.(cpp|h)$")

# Add executable
add_executable(DODO ${SOURCES} ${THIRD_PARTY_SOURCES})

# Include directories and link libraries
target_include_directories(DODO PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB THIRD_PARTY_INCLUDE_DIRS RELATIVE ${THIRD_PARTY_DIR} ${THIRD_PARTY_DIR}/*/include)
foreach(include_dir ${THIRD_PARTY_INCLUDE_DIRS})
    include_directories(${THIRD_PARTY_DIR}/${include_dir})
endforeach()

# Add Vulkan as dependency
find_package(Vulkan REQUIRED)
target_include_directories(DODO PUBLIC ${Vulkan_INCLUDE_DIRS})

# Set up precompiled headers
target_precompile_headers(DODO PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

# Link Vulkan libraries
target_link_libraries(DODO PUBLIC ${Vulkan_LIBRARIES})

# Add platform-specific definitions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(DODO PRIVATE _WIN32)
endif()

# Set debug and release symbols
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_RELEASE")