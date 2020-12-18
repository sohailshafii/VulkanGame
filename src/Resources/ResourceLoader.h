#pragma once

#include <map>
#include <string>
#include <memory>
#include "vulkan/vulkan.h"

class ShaderLoader;
class TextureCreator;
class GfxDeviceManager;
class LogicalDeviceManager;
class Model;

// Responsible for loading and caching any
// resources used, like textures, shaders,
// sounds, et cetera.
class ResourceLoader {
public:
	ResourceLoader();
	~ResourceLoader();

	std::shared_ptr<ShaderLoader> GetShader(std::string path, VkDevice device);
	std::shared_ptr<TextureCreator> GetTexture(const std::string& path,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		VkCommandPool commandPool);
	

	std::shared_ptr<TextureCreator> BuildRawTexture(std::string textureName,
		unsigned char* pixels,
		int texWidth, int texHeight, int bytesPerPixel,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		VkCommandPool commandPool);
	std::shared_ptr<TextureCreator> GetRawTexture(std::string textureName);

	std::shared_ptr<Model> GetModel(const std::string& path);

private:
	std::map<std::string, std::shared_ptr<ShaderLoader>> shadersLoaded;
	std::map<std::string, std::shared_ptr<TextureCreator>> texturesLoaded;
	std::map<std::string, std::shared_ptr<Model>> modelsLoaded;
};
