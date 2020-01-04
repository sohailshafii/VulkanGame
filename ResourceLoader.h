#pragma once

#include <map>
#include <string>
#include "vulkan/vulkan.h"
#include "ShaderLoader.h"

// Responsible for loading and caching any
// resources used, like textures, shaders,
// sounds, et cetera.
class ResourceLoader {
public:
	ResourceLoader();
	~ResourceLoader();

	ShaderLoader* getShader(std::string path, VkDevice device);

private:

	std::map<std::string, ShaderLoader*> shadersLoaded;
};
