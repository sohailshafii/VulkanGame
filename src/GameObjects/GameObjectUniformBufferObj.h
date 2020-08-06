#pragma once

#include "vulkan/vulkan.h"
#include <memory>

class GfxDeviceManager;
class LogicalDeviceManager;

class GameObjectUniformBufferObj {
public:
	GameObjectUniformBufferObj(std::shared_ptr<LogicalDeviceManager> logicalDeviceManager, GfxDeviceManager* gfxDeviceManager,
		int bufferSize);
	~GameObjectUniformBufferObj();

	VkBuffer GetUniformBuffer() const {
		return uniformBuffer;
	}
	
	VkDeviceMemory GetUniformBufferMemory() const {
		return uniformBufferMemory;
	}

	int GetBufferSize() const {
		return bufferSize;
	}

private:
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	int bufferSize;
};
