#pragma once

#include <map>
#include <string>
#include "ShaderModule.h"

// Responsible for loading and caching any
// resources used, like textures, shaders,
// sounds, et cetera.
class ResourceLoader {
public:
	ResourceLoader();
	~ResourceLoader();

	//ShaderModule& getShader(std::string path);

private:

	std::map<std::string, ShaderModule> shadersLoaded;
};
