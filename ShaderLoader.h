#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <vector>

class ShaderLoader {
public:
	ShaderLoader(const std::string& path, VkDevice device);
	~ShaderLoader();

	VkShaderModule getVkShaderModule() const {
		return shaderModule;
	}

	ShaderLoader& operator=(const ShaderLoader& rhs);

private:
	VkShaderModule shaderModule;
	VkDevice device;

	std::vector<char> readFile(const std::string& path);
	VkShaderModule assembleShaderModule(const std::vector<char>& code);
};