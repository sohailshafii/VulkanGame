#pragma once

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan/vulkan.h"
#include "SceneManagement/SceneLoader.h"
#include "SceneManagement/Scene.h"

class Scene;
class GraphicsEngine;
class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;
class Camera;

class GameEngine {
public:
	enum class GameMode : char { Menu = 0, Game };

	GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const & logicalDeviceManager,
		ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
		VkCommandPool commandPool);
	~GameEngine();

	void CreateMenuObjects(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);

	void UpdateGameMode(GameMode newGameMode);
	void RecreateGraphicsEngineForNewSwapchain(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
		VkCommandPool commandPool);

	void UpdateFrame(float time, float deltaTime, uint32_t imageIndex,
		GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
		std::vector<VkFence> const & inFlightFences);

	GraphicsEngine* GetGraphicsEngine() {
		return graphicsEngine;
	}

	Camera* GetCamera() {
		return mainCamera.get();
	}

	void SpawnGameObject(Scene::SpawnType spawnType,
		glm::vec3 const& spawnPosition,
		glm::vec3 const& forwardDir) {
		mainGameScene->SpawnGameObject(
			Scene::SpawnType::Bullet, spawnPosition,
			forwardDir);
	}

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

	void RemoveGameObjects(std::vector<GameObject*>& gameObjectsToRemove,
		std::vector<VkFence> const& inFlightFences);
	void RemoveGameObjects(std::vector<std::shared_ptr<GameObject>>
		& gameObjectsToRemove,
		std::vector<VkFence> const& inFlightFences);
};
