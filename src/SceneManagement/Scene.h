#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan/vulkan.h"
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject;
class ResourceLoader;
class GfxDeviceManager;
class LogicalDeviceManager;
class GraphicsEngine;

class Scene
{
public:
	enum SpawnType { Pawn = 0, Bullet };
	enum SceneMode { Menu = 0, Game };

	Scene(SceneMode currentSceneMode, ResourceLoader* resourceLoader,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);
	~Scene();

	void CreateGraphicsEngine(VkSurfaceKHR surface, GLFWwindow* window);

	void UpdateSceneMode(SceneMode newSceneMode);
	
	void AddGameObject(std::shared_ptr<GameObject> const & newGameObject);
	
	GameObject* GetGameObject(unsigned int index);
	
	void ClearGameObjects() {
		gameObjects.clear();
	}
	
	std::vector<std::shared_ptr<GameObject>>& GetGameObjects() {
		return gameObjects;
	}

	void RemoveGameObject(GameObject* gameObjectToRemove);

	void RemoveGameObject(std::shared_ptr<GameObject> const & gameObjectToRemove);

	void RemoveGameObjects(std::vector<GameObject*> const& gameObjectsToRemove);

	void RemoveGameObjects(std::vector<std::shared_ptr<GameObject>> const &
		gameObjectsToRemove);

	std::shared_ptr<GameObject> GetPlayerGameObject();

	void SpawnGameObject(SpawnType spawnType,
		glm::vec3 const & spawnPosition,
		glm::vec3 const& forwardDir);

	void Update(float time, float deltaTime, uint32_t imageIndex,
		glm::mat4 const& viewMatrix, VkExtent2D swapChainExtent);
	
private:
	SceneMode currentSceneMode;
	GraphicsEngine* graphicsEngine;

	std::vector<std::shared_ptr<GameObject>> gameObjects;
	std::vector<std::shared_ptr<GameObject>> upcomingGameObjects;

	// we don't own these pointers; should be shared ptrs ideally
	ResourceLoader* resourceLoader;
	GfxDeviceManager* gfxDeviceManager;
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	VkCommandPool commandPool;

	void SpawnPawnGameObject(glm::vec3 const & spawnPosition,
							 glm::vec3 const& forwardDirection);
	void SpawnBulletGameObject(glm::vec3 const& spawnPosition,
							   glm::vec3 const& forwardDir);
};

