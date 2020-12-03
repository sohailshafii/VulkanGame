#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <memory>

class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;
class Model;

class MenuObject {
public:
	MenuObject(std::string const & menuText,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);
private:
	static std::shared_ptr<Model> model;
};
