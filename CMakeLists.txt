cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(VulkanGame)

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src SOURCE_FILES) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/Math SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects/FontObjects SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects/Mothership SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects/Msc SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects/Player SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/GameObjects/Turrets SOURCE_FILES)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/SceneManagement SOURCE_FILES) 
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/Resources SOURCE_FILES)  
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/src/Rendering SOURCE_FILES) 

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

find_package(FreeType REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
target_link_libraries(VulkanGame ${FREETYPE_LIBRARIES})

target_include_directories(VulkanGame
	PRIVATE
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}/src/"
		"${PROJECT_SOURCE_DIR}/src/Math"
		"${PROJECT_SOURCE_DIR}/src/GameObjects"
		"${PROJECT_SOURCE_DIR}/src/GameObjects/FontObjects"
		"${PROJECT_SOURCE_DIR}/src/GameObjects/Mothership"
		"${PROJECT_SOURCE_DIR}/src/GameObjects/Msc"
		"${PROJECT_SOURCE_DIR}/src/GameObjects/Player"
		"${PROJECT_SOURCE_DIR}/src/GameObjects/Turrets"
		"${PROJECT_SOURCE_DIR}/src/SceneManagement"
		"${PROJECT_SOURCE_DIR}/src/Resources"
		"${PROJECT_SOURCE_DIR}/src/ThirdParty"
		"${PROJECT_SOURCE_DIR}/src/Rendering")


