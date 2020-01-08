#pragma once

#include <map>
#include <string>
#include <memory>
#include "vulkan/vulkan.h"

class ShaderLoader;
class ImageTextureLoader;
class GfxDeviceManager;
class LogicalDeviceManager;

// Responsible for loading and caching any
// resources used, like textures, shaders,
// sounds, et cetera.
class ResourceLoader {
public:
	ResourceLoader();
	~ResourceLoader();

	std::shared_ptr<ShaderLoader> getShader(std::string path, VkDevice device);
	std::shared_ptr<ImageTextureLoader> getTexture(const std::string& path,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		VkCommandPool commandPool);

private:

	std::map<std::string, std::shared_ptr<ShaderLoader>> shadersLoaded;
	std::map<std::string, std::shared_ptr<ImageTextureLoader>> texturesLoaded;
};
