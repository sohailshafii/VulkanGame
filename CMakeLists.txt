cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(VulkanGame)

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src SOURCE_FILES) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects SOURCE_FILES) 

add_executable(VulkanGame ${SOURCE_FILES})

find_package(Vulkan REQUIRED)
target_include_directories(VulkanGame PRIVATE Vulkan::Vulkan)
target_link_libraries(VulkanGame Vulkan::Vulkan)

FIND_PACKAGE(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
target_link_libraries(VulkanGame ${GLM_LIBRARY})

find_package(GLFW REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(VulkanGame ${GLFW_LIBRARY})

target_include_directories(VulkanGame
	PRIVATE
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}/src/"
		"${PROJECT_SOURCE_DIR}/src/GameObjects"
		"${PROJECT_SOURCE_DIR}/src/ThirdParty")


