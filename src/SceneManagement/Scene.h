#pragma once

#include "vulkan/vulkan.h"
#include <memory>
#include <vector>
#include <string>

class GameObject;
class ResourceLoader;
class GfxDeviceManager;
class LogicalDeviceManager;

class Scene
{
public:
	Scene();
	~Scene();
	
	void AddGameObject(std::shared_ptr<GameObject> const & newGameObject);
	
	GameObject* GetGameObject(unsigned int index);
	
	void ClearGameObjects()
	{
		gameObjects.clear();
	}
	
	std::vector<std::shared_ptr<GameObject>>& GetGameObjects()
	{
		return gameObjects;
	}

	void SpawnGameObject(std::string const& gameObjectName);
	
private:
	std::vector<std::shared_ptr<GameObject>> gameObjects;
	
	std::shared_ptr<GameObject> NewGameObject();

	// we don't own these pointers; should be shared ptrs ideally
	ResourceLoader* resourceLoader;
	GfxDeviceManager* gfxDeviceManager;
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	VkCommandPool commandPool;
};

