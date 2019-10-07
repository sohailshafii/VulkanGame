#pragma once

#include "vulkan/vulkan.h"
#include "GfxDeviceManager.h"
#include <string>

class Pipeline {
public:
	Pipeline(const std::string& vertShaderPath,
		const std::string& fragShaderPath, VkDevice device,
		VkExtent2D swapChainExtent, GfxDeviceManager *gfxDeviceManager);
	~Pipeline();
};
