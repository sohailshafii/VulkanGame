#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <vector>

class ShaderModule {
public:
	ShaderModule(const std::string& path, VkDevice device);
	~ShaderModule();

	VkShaderModule getVkShaderModule() const {
		return shaderModule;
	}

private:
	VkShaderModule shaderModule;
	VkDevice device;

	std::vector<char> readFile(const std::string& path);
	VkShaderModule assembleShaderModule(const std::vector<char>& code);
};