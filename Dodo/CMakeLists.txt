cmake_minimum_required(VERSION 3.27.1)

# Vulkan:
set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
find_package(Vulkan REQUIRED)
message("Vulkan SDK found at: ${VULKAN_SDK}")

# Additional Vulkan libs:
find_library(SHADERC_COMBINEDD_LIB shaderc_combinedd HINTS "${VULKAN_SDK_PATH}/Lib")

# Dodo source files & app exe:
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
add_executable(Dodo ${SOURCES})

# Additional include dirs:
target_include_directories(Dodo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS})

# Link libs:
target_link_libraries(Dodo PRIVATE ${Vulkan_LIBRARIES} spdlog yaml-cpp ${SHADERC_COMBINEDD_LIB})

target_precompile_headers(Dodo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

# Platform:
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(Dodo PRIVATE _WIN32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(Dodo PRIVATE __linux__)
endif()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_RELEASE")
