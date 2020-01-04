
#include "ResourceLoader.h"

ResourceLoader::ResourceLoader() {

}

ResourceLoader::~ResourceLoader() {
	for (auto& shaderObjPair : shadersLoaded) {
		delete shaderObjPair.second;
	}
	shadersLoaded.clear();
}


ShaderLoader* ResourceLoader::getShader(std::string path, VkDevice device) {
	auto& foundShaderItr = shadersLoaded.find(path);
	if (foundShaderItr != shadersLoaded.cend()) {
		return foundShaderItr->second;
	}

	auto* newShader = new ShaderLoader(path, device);
	shadersLoaded[path] = newShader;
	return newShader;
}
