#pragma once

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan/vulkan.h"
#include "SceneManagement/SceneLoader.h"

class Scene;
class GraphicsEngine;
class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;
class Camera;

class GameEngine {
public:
	enum GameMode { Menu = 0, Game };

	GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const & logicalDeviceManager,
		ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
		VkCommandPool commandPool);
	~GameEngine();

	void UpdateGameMode(GameMode newGameMode);

private:
	GameMode currentGameMode;
	std::shared_ptr<Camera> mainCamera;
	Scene* mainGameScene;
	GraphicsEngine* graphicsEngine;

	SceneLoader::SceneSettings CreateSceneAndReturnSettings(
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool,
		VkSurfaceKHR surface, GLFWwindow* window);
	void CreatePlayerGameObject(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);
};
