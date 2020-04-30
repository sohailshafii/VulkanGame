#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class VulkanInstance;
class GfxDeviceManager;

class LogicalDeviceManager {
public:
	LogicalDeviceManager(const GfxDeviceManager *gfxDeviceManager,
		const VulkanInstance *instance, const VkSurfaceKHR surface,
		const std::vector<const char*>& deviceExtensions,
		bool enableValidationLayers);
	~LogicalDeviceManager();

	VkDevice GetDevice() {
		return device;
	}

	VkQueue GetGraphicsQueue() {
		return graphicsQueue;
	}

	VkQueue GetPresentQueue() {
		return presentQueue;
	}

private:
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
};
