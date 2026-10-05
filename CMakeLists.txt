cmake_minimum_required(VERSION 3.14)
project(Engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
find_package(Vulkan REQUIRED)
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
	FetchContent_Declare(
		glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG 3.4
	)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(glfw)
endif()
find_package(OpenSSL QUIET)
if(NOT OpenSSL_FOUND)
	FetchContent_Declare(
		openssl
		GIT_REPOSITORY https://github.com/openssl/openssl.git
		GIT_TAG openssl-3.5.4
	)
	FetchContent_MakeAvailable(openssl)
endif()
find_package(glm QUIET)
if(NOT glm_FOUND)
	FetchContent_Declare(
		glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		GIT_TAG 1.0.1
	)
	FetchContent_MakeAvailable(glm)
endif()
file(GLOB_RECURSE SOURCE_FILES
	"engine_code/*.c++"
	"engine_code/*.hpp"
	"engine_code/*.cpp"
	"include/tiny_obj_loader.h"
)
execute_process(
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile.sh
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	RESULT_VARIABLE RESULT
)
if(NOT RESULT EQUAL 0)
	message(FATAL_ERROR "Shader compilation failed")
endif()
add_executable(Engine ${SOURCE_FILES})
target_include_directories(Engine
	PRIVATE
		engine_code
)
if(WIN32)
	target_compile_definitions(Engine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	target_compile_definitions(Engine PRIVATE VK_USE_PLATFORM_MACOS_MVK)
elseif(UNIX)
	target_compile_definitions(Engine PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()
target_link_libraries(Engine
	PRIVATE
		Vulkan::Vulkan
		glfw
		OpenSSL::SSL
		OpenSSL::Crypto
		glm::glm
)