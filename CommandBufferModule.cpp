
#include "CommonBufferModule.h"
#include <stdexcept>

CommandBufferModule::CommandBufferModule(size_t numBuffers,
	VkDevice logicalDevice, VkCommandPool commandPool) {
	commandBuffers.resize(numBuffers);
	this->logicalDevice = logicalDevice;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data())
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

CommandBufferModule::~CommandBufferModule() {
	vkFreeCommandBuffers(logicalDevice, commandPool,
		static_cast<uint32_t>(commandBuffers.size()),
		commandBuffers.data());
}

