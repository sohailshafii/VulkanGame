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
class MenuObject;
class Model;
class Material;

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

	void SpawnGameObject(Scene::SpawnType spawnType,
		glm::vec3 const& spawnPosition,
		glm::vec3 const& forwardDir) {
		mainGameScene->SpawnGameObject(
			Scene::SpawnType::Bullet, spawnPosition,
			forwardDir);
	}

	void ProcessMouse(float xoffset, float yoffset);
	void ProcessInput(GLFWwindow* window,
		float frameTime, float latestFrameTime);
	void ProcessKeyCallback(GLFWwindow* window, int key,
		int scancode, int action, int mods);

private:
	GameMode currentGameMode;
	std::shared_ptr<Camera> mainCamera;
	Scene* mainGameScene;
	GraphicsEngine* graphicsEngine;

	float lastFireTime;
	float fireInterval;

	std::shared_ptr<Material> menuMaterial;
	std::vector<std::shared_ptr<MenuObject>> menuObjects;
	size_t currentSelectedMenuObject;
	std::vector<std::shared_ptr<GameObject>> normalGameObjects;
	std::shared_ptr<class TextureCreator> fontTextureSheet;
	class FontTextureBuffer* fontTextureBuffer;

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

	void HandleMainMenuControls(GLFWwindow* window, int key,
		int scancode, int action, int mods);
	void SelectNextMenuObject(bool moveUp);

	void HandleMainGameControls(GLFWwindow* window, float frameTime, float latestFrameTime);
	void FireMainCannon(float latestFrameTime);
};
