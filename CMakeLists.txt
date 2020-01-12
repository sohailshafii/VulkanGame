cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(VulkanGame)

set (CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/main.cpp)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR} SOURCE_FILES) 

add_executable(VulkanGame ${SOURCE_FILES})

find_package(Vulkan REQUIRED)
target_include_directories(VulkanGame PRIVATE Vulkan::Vulkan)
target_link_libraries(VulkanGame Vulkan::Vulkan)

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})

find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS})

target_include_directories(VulkanGame
	PRIVATE
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}")


