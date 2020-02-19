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
	objModel(model), logicalDeviceManager(logicalDeviceManager), descriptorPool(nullptr) {
	CreateVertexBuffer(model->GetVertices(), gfxDeviceManager, commandPool);
	CreateIndexBuffer(model->GetIndices(), gfxDeviceManager, commandPool);
}

void GameObject::CreateVertexBuffer(const std::vector<Vertex>& vertices,
									GfxDeviceManager *gfxDeviceManager,
									VkCommandPool commandPool) {
	VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void*data;
	vkMapMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory);

	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	Common::CopyBuffer(logicalDeviceManager.get(), commandPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(logicalDeviceManager->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory, nullptr);
}

GameObject::~GameObject() {
	vkDestroyBuffer(logicalDeviceManager->GetDevice(), indexBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), indexBufferMemory, nullptr);
	vkDestroyBuffer(logicalDeviceManager->GetDevice(), vertexBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), vertexBufferMemory, nullptr);
	CleanUpUniformBuffers();
	CleanUpDescriptorPool();
}

void GameObject::CreateIndexBuffer(const std::vector<uint32_t>& indices,
								   GfxDeviceManager *gfxDeviceManager,
								   VkCommandPool commandPool) {
	VkDeviceSize bufferSize = sizeof(indices[0])*indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory);

	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	Common::CopyBuffer(logicalDeviceManager.get(), commandPool, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(logicalDeviceManager->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory, nullptr);
}

void GameObject::CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager, size_t numSwapChainImages) {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	
	uniformBuffers.resize(numSwapChainImages);
	uniformBuffersMemory.resize(numSwapChainImages);
	for (size_t i = 0; i < numSwapChainImages; i++) {
		Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

void GameObject::CleanUpUniformBuffers() {
	size_t numBuffers = uniformBuffers.size();
	for (size_t i = 0; i < numBuffers; i++) {
		vkDestroyBuffer(logicalDeviceManager->GetDevice(), uniformBuffers[i], nullptr);
		vkFreeMemory(logicalDeviceManager->GetDevice(), uniformBuffersMemory[i], nullptr);
	}
	uniformBuffers.clear();
}

void GameObject::CreateCommandBuffers(GfxDeviceManager* gfxDeviceManager,
							size_t numSwapChainImages) {
	CleanUpUniformBuffers();
	CreateUniformBuffers(gfxDeviceManager, numSwapChainImages);
}

void GameObject::CreateDescriptorPoolAndSets(size_t numSwapChainImages,
								 VkDescriptorSetLayout descriptorSetLayout,
								 VkImageView textureImageView, VkSampler textureSampler) {
	CleanUpDescriptorPool();
	CreateDescriptorPool(numSwapChainImages);
	CreateDescriptorSets(descriptorSetLayout, textureImageView, textureSampler,
						 numSwapChainImages);
}

// TODO: use push constants, more efficient
void GameObject::UpdateUniformBuffer(uint32_t imageIndex, const glm::mat4& viewMatrix,
									 VkExtent2D swapChainExtent) {
	UniformBufferObject ubo = {};
	ubo.model = modelMatrix;
	ubo.view = viewMatrix;
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1; // flip Y -- opposite of opengl

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), uniformBuffersMemory[imageIndex], 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(logicalDeviceManager->GetDevice(), uniformBuffersMemory[imageIndex]);
}

void GameObject::CreateDescriptorPool(size_t numSwapChainImages) {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

	if (vkCreateDescriptorPool(logicalDeviceManager->GetDevice(), &poolInfo,
		nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void GameObject::CreateDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
	VkImageView textureImageView, VkSampler textureSampler,
	size_t numSwapChainImages) {
	std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages,
		descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(numSwapChainImages);
	if (vkAllocateDescriptorSets(logicalDeviceManager->GetDevice(), &allocInfo,
		descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}
	
	for (size_t i = 0; i < numSwapChainImages; ++i) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = GetUniformBuffer(i);
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(logicalDeviceManager->GetDevice(), static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(), 0,
			nullptr);
	}
}

void GameObject::CleanUpDescriptorPool() {
	if (descriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDeviceManager->GetDevice(), descriptorPool, nullptr);
		descriptorPool = nullptr;
	}
}
