
#include "Resources/ResourceLoader.h"
#include "Resources/ShaderLoader.h"
#include "Resources/TextureCreator.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Model.h"

ResourceLoader::ResourceLoader() {

}

ResourceLoader::~ResourceLoader() {
	shadersLoaded.clear();
	texturesLoaded.clear();
}

std::shared_ptr<ShaderLoader> ResourceLoader::GetShader(std::string path, VkDevice device) {
	auto foundShaderItr = shadersLoaded.find(path);
	if (foundShaderItr != shadersLoaded.cend()) {
		return foundShaderItr->second;
	}

	auto newShader = std::make_shared<ShaderLoader>(path, device);
	shadersLoaded[path] = newShader;
	return newShader;
}

std::shared_ptr<TextureCreator> ResourceLoader::GetTexture(const std::string& path,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	VkCommandPool commandPool) {
	auto foundTexturItr = texturesLoaded.find(path);
	if (foundTexturItr != texturesLoaded.cend()) {
		return foundTexturItr->second;
	}

	auto newTexture = std::make_shared<TextureCreator>(path, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	texturesLoaded[path] = newTexture;
	return newTexture;
}

std::shared_ptr<Model> ResourceLoader::GetModel(const std::string& path) {
	auto foundModelItr = modelsLoaded.find(path);
	if (foundModelItr != modelsLoaded.cend()) {
		return foundModelItr->second;
	}

	auto newModel = std::make_shared<Model>(path);
	modelsLoaded[path] = newModel;
	return newModel;
}
