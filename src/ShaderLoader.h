#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <vector>

class ShaderLoader {
public:
	ShaderLoader(const std::string& path, VkDevice device);
	~ShaderLoader();

	VkShaderModule GetVkShaderModule() const {
		return shaderModule;
	}

	ShaderLoader& operator=(const ShaderLoader& rhs);

private:
	VkShaderModule shaderModule;
	VkDevice device;

	std::vector<char> ReadFile(const std::string& path);
	VkShaderModule AssembleShaderModule(const std::vector<char>& code);
};
