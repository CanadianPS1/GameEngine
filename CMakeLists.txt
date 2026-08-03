cmake_minimum_required(VERSION 3.10)
project(Engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenSSL REQUIRED)
file(GLOB_RECURSE SOURCE_FILES
    "engine_code/*.c++"
    "engine_code/*.hpp"
    "engine_code/*.cpp"
    "include/tiny_obj_loader.h"
)
add_executable(Engine ${SOURCE_FILES})
target_include_directories(Engine
    PRIVATE
        engine_code
)
target_link_libraries(Engine
    PRIVATE
        Vulkan::Vulkan
        glfw
        OpenSSL::SSL
        OpenSSL::Crypto
)