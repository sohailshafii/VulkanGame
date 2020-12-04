#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include <vector>

class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;
class Model;
class Material;
class GameObject;

class MenuObject {
public:
	MenuObject(std::string const & menuText,
		std::shared_ptr<Model> & model,
		std::shared_ptr<Material> & gameObjectMaterial,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);
private:
	std::shared_ptr<Model> model;
	std::shared_ptr<Material> gameObjectMaterial;
	std::vector<std::shared_ptr<GameObject>> textGameObjects;
};
