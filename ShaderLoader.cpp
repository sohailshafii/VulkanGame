#include "ShaderLoader.h"
#include <fstream>
#include <iostream>

ShaderLoader::ShaderLoader(const std::string& path, VkDevice device) {
	this->device = device;
	auto shaderCode = readFile(path);
	shaderModule = assembleShaderModule(shaderCode);
}

ShaderLoader::~ShaderLoader() {
	vkDestroyShaderModule(device, shaderModule, nullptr);
}

ShaderLoader& ShaderLoader::operator=(const ShaderLoader& rhs) {
	if (this != &rhs) {
		shaderModule = rhs.shaderModule;
		device = rhs.device;
	}
	return *this;
}

std::vector<char> ShaderLoader::readFile(const std::string& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	std::cout << "Number of bytes to read: " << fileSize << ".\n";

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule ShaderLoader::assembleShaderModule(const
	std::vector<char>& code) {
	// need to make sure data satisfies alignment requirements of
	// uint32_t. fortunately, std::vector's default allocator
	// ensures data satisfies worst case alignment requirements
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr,
		&shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}
