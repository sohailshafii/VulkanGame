#include "GameObjectUniformBufferObj.h"
#include "LogicalDeviceManager.h"
#include "GfxDeviceManager.h"
#include "Common.h"

GameObjectUniformBufferObj::GameObjectUniformBufferObj(
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	GfxDeviceManager* gfxDeviceManager,
	int bufferSize) {
	this->logicalDeviceManager = logicalDeviceManager;
	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
	this->bufferSize = bufferSize;
}

GameObjectUniformBufferObj::~GameObjectUniformBufferObj() {
	vkDestroyBuffer(logicalDeviceManager->GetDevice(), uniformBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), uniformBufferMemory, nullptr);
}
