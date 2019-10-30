#pragma once

#include <vector>
#include "vulkan/vulkan.h"

class CommandBufferModule {
public:
	CommandBufferModule(size_t numBuffers,
		VkDevice logicalDevice, VkCommandPool commandPool);
	~CommandBufferModule();

	std::vector<VkCommandBuffer>& getCommandBuffers() {
		return commandBuffers;
	}

private:
	VkDevice logicalDevice;

	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandPool commandPool;
};
