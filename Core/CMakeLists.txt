cmake_minimum_required(VERSION 3.28.1)
project(CoreEngine VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add includes
include_directories(Source)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

# Add third-party includes
include_directories(ThirdParty/spdlog/include)

# Add source files
file(GLOB_RECURSE HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

# Exclude precompiled headers from source files
list(FILTER HEADER_FILES EXCLUDE REGEX ".*pch.h$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*pch.cpp$")

# Add precompiled headers
set(PCH_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Source/pch.h)
add_library(CoreEngine_PCH STATIC ${PCH_FILE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Source/pch.cpp)
set_target_properties(CoreEngine_PCH PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(CoreEngine_PCH PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Core)
target_precompile_headers(CoreEngine_PCH PRIVATE ${PCH_FILE_PATH})

# Create static library
add_library(CoreEngine STATIC ${HEADER_FILES} ${SOURCE_FILES})

# Link Vulkan libraries
target_link_libraries(CoreEngine ${VULKAN_LIBRARIES})

# Add platform-specific definitions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(CoreEngine PRIVATE _WIN32)
endif()

# Set debug and release symbols
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_RELEASE")

# Link with precompiled header
target_link_libraries(CoreEngine PUBLIC CoreEngine_PCH)