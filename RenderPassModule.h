#pragma once

#include "vulkan/vulkan.h"

class RenderPassModule {
public:
	RenderPassModule(VkDevice device, VkPhysicalDevice physicalDevice, VkFormat swapChainImageFormat,
		VkSampleCountFlagBits msaaSamples);
	~RenderPassModule();

	VkRenderPass GetRenderPass() { return renderPass; }

private:
	VkRenderPass renderPass;
	VkDevice device;
};
