#pragma once

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

class Scene
{
public:
	Scene(ResourceLoader* resourceLoader,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);
	~Scene();
	
	void AddGameObject(std::shared_ptr<GameObject> const & newGameObject);
	
	GameObject* GetGameObject(unsigned int index);
	
	void ClearGameObjects() {
		gameObjects.clear();
	}
	
	std::vector<std::shared_ptr<GameObject>>& GetGameObjects() {
		return gameObjects;
	}

	std::shared_ptr<GameObject> GetPlayerGameObject();

	void SpawnGameObject(std::string const& gameObjectName);

	void Update(float time, float deltaTime, uint32_t imageIndex,
		glm::mat4 const& viewMatrix, VkExtent2D swapChainExtent);
	
private:
	std::vector<std::shared_ptr<GameObject>> gameObjects;
	std::vector<std::shared_ptr<GameObject>> upcomingGameObjects;

	// we don't own these pointers; should be shared ptrs ideally
	ResourceLoader* resourceLoader;
	GfxDeviceManager* gfxDeviceManager;
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	VkCommandPool commandPool;
};

