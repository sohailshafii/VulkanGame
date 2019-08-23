#pragma once

#include "vulkan/vulkan.h"

class GfxDeviceManager {
public:
	GfxDeviceManager(const VkInstance& vkInstance);
	~GfxDeviceManager();
private:
	void pickPhysicalDevice(const VkInstance& vkInstance);
	bool isDeviceSuitable(VkPhysicalDevice device);

	VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice device);

	VkPhysicalDevice physicalDevice;
	VkSampleCountFlagBits msaaSamples;
};
