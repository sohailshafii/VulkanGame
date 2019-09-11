#pragma once

#include "vulkan/vulkan.h"

class LogicalDeviceManager;

class Common {
public:
	static VkImageView Common::createImageView(VkImage image, VkFormat format,
		VkImageAspectFlags aspectFlags, uint32_t mipLevels,
		LogicalDeviceManager* logicalDeviceManager);
};
