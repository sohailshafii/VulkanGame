#pragma once

#include <string>
#include "vulkan/vulkan.h"

class ResourceLoader;
class GfxDeviceManager;
class LogicalDeviceManager;

class SceneLoader
{
public:
	static void DeserializeJSONFileIntoScene(
		ResourceLoader* resourceLoader,
		GfxDeviceManager *gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager>const& logicalDeviceManager,
		VkCommandPool commandPool,
		class Scene* scene,
		const std::string& jsonFilePath);

private:

};
