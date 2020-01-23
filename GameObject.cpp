#include "GameObject.h"
#include "Model.h"
#include "Vertex.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Common.h"

GameObject::GameObject(std::shared_ptr<Model> model,
					   GfxDeviceManager *gfxDeviceManager,
					   std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
					   VkCommandPool commandPool) :
	objModel(model), logicalDeviceManager(logicalDeviceManager) {
}

void GameObject::createVertexBuffer(const std::vector<Vertex>& vertices,
									GfxDeviceManager *gfxDeviceManager,
									VkCommandPool commandPool) {
	VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void*data;
	vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

	Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	Common::copyBuffer(logicalDeviceManager.get(), commandPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(logicalDeviceManager->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, nullptr);
}

GameObject::~GameObject() {
	vkDestroyBuffer(logicalDeviceManager->getDevice(), indexBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), indexBufferMemory, nullptr);
	vkDestroyBuffer(logicalDeviceManager->getDevice(), vertexBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), vertexBufferMemory, nullptr);
}

void GameObject::createIndexBuffer(const std::vector<uint32_t>& indices,
								   GfxDeviceManager *gfxDeviceManager,
								   VkCommandPool commandPool) {
	VkDeviceSize bufferSize = sizeof(indices[0])*indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

	Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	Common::copyBuffer(logicalDeviceManager.get(), commandPool, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(logicalDeviceManager->getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, nullptr);
}
