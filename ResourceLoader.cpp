
#include "ResourceLoader.h"
#include "ShaderLoader.h"
#include "ImageTextureLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "ModelLoader.h"

ResourceLoader::ResourceLoader() {

}

ResourceLoader::~ResourceLoader() {
	shadersLoaded.clear();
	texturesLoaded.clear();
}

std::shared_ptr<ShaderLoader> ResourceLoader::getShader(std::string path, VkDevice device) {
	auto foundShaderItr = shadersLoaded.find(path);
	if (foundShaderItr != shadersLoaded.cend()) {
		return foundShaderItr->second;
	}

	auto newShader = std::make_shared<ShaderLoader>(path, device);
	shadersLoaded[path] = newShader;
	return newShader;
}

std::shared_ptr<ImageTextureLoader> ResourceLoader::getTexture(const std::string& path,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	VkCommandPool commandPool) {
	auto foundTexturItr = texturesLoaded.find(path);
	if (foundTexturItr != texturesLoaded.cend()) {
		return foundTexturItr->second;
	}

	auto newTexture = std::make_shared<ImageTextureLoader>(path, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	texturesLoaded[path] = newTexture;
	return newTexture;
}

std::shared_ptr<ModelLoader> ResourceLoader::getModel(const std::string& path) {
	auto foundModelItr = modelsLoaded.find(path);
	if (foundModelItr != modelsLoaded.cend()) {
		return foundModelItr->second;
	}

	auto newModel = std::make_shared<ModelLoader>(path);
	modelsLoaded[path] = newModel;
	return newModel;
}
