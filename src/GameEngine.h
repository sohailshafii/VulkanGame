#pragma once

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan/vulkan.h"

class Scene;
class GraphicsEngine;
class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;

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
	Scene* mainGameScene;
	GraphicsEngine* graphicsEngine;

	void CreateSceneAndGameObjects();
};
