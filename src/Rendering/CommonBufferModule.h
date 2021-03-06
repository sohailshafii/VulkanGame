#pragma once

#include <vector>
#include "vulkan/vulkan.h"

class CommandBufferModule {
public:
	CommandBufferModule(size_t numBuffers,
		VkDevice logicalDevice, VkCommandPoolCreateInfo poolInfo);
	~CommandBufferModule();

	std::vector<VkCommandBuffer>& GetCommandBuffers() {
		return commandBuffers;
	}

private:
	VkDevice logicalDevice;

	std::vector<VkCommandBuffer> commandBuffers;
	VkCommandPool commandPool;
};
