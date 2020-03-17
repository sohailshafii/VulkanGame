#include "GameObject.h"
#include "Model.h"
#include "Vertex.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Common.h"
#include "ImageTextureLoader.h"

GameObject::GameObject(std::shared_ptr<Model> const& model,
					   GfxDeviceManager *gfxDeviceManager,
					   std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
					   std::shared_ptr<ImageTextureLoader> const& textureLoader,
					   VkCommandPool commandPool,
					   DescriptorSetFunctions::MaterialType materialType) :
	objModel(model), textureLoader(textureLoader), logicalDeviceManager(logicalDeviceManager),
	descriptorPool(nullptr), materialType(materialType) {
	SetupShaderNames();
	descriptorSetLayout = DescriptorSetFunctions::CreateDescriptorSetLayout(logicalDeviceManager->GetDevice(), materialType);
	CreateVertexBuffer(model->BuildAndReturnVertsPosColorTexCoord(), gfxDeviceManager, commandPool);
	CreateIndexBuffer(model->GetIndices(), gfxDeviceManager, commandPool);
}

// TODO: allow custom verts
void GameObject::CreateVertexBuffer(const std::vector<VertexPosColorTexCoord>& vertices,
									GfxDeviceManager *gfxDeviceManager,
									VkCommandPool commandPool) {
	VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void *data;
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
	vkDestroyDescriptorSetLayout(logicalDeviceManager->GetDevice(), descriptorSetLayout, nullptr);
	CleanUpUniformBuffers();
	CleanUpDescriptorPool();
}

void GameObject::SetupShaderNames() {
	switch (materialType) {
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
			vertexShaderName = "UnlitTintedTexturedVert.spv";
			fragmentShaderName = "UnlitTintedTexturedFrag.spv";
			break;
		case DescriptorSetFunctions::SimpleLambertian:
			vertexShaderName = "SimpleLambertianVert.spv";
			fragmentShaderName = "SimpleLambertianFrag.spv";
			break;
	}
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
	VkDeviceSize bufferSizeVert = sizeof(UniformBufferObjectVert);
	VkDeviceSize bufferSizeLighting = sizeof(UniformBufferObjectLighting);
	
	for (size_t i = 0; i < numSwapChainImages; i++) {
		uniformBuffersVert.push_back(new GameObjectUniformBufferObj(logicalDeviceManager, gfxDeviceManager, bufferSizeVert));
		// TODO: fix frag
		uniformBuffersFrag.push_back(new GameObjectUniformBufferObj(logicalDeviceManager, gfxDeviceManager, bufferSizeLighting));
	}
}

void GameObject::CleanUpUniformBuffers() {
	size_t numBuffers = uniformBuffersVert.size();
	
	for (size_t bufferIndex = 0; bufferIndex < numBuffers;
		 bufferIndex++)
	{
		delete uniformBuffersVert[bufferIndex];
		delete uniformBuffersFrag[bufferIndex];
	}
	
	uniformBuffersVert.clear();
	uniformBuffersFrag.clear();
}

void GameObject::CreateCommandBuffers(GfxDeviceManager* gfxDeviceManager,
							size_t numSwapChainImages) {
	CleanUpUniformBuffers();
	CreateUniformBuffers(gfxDeviceManager, numSwapChainImages);
}

void GameObject::CreateDescriptorPoolAndSets(size_t numSwapChainImages) {
	CleanUpDescriptorPool();
	CreateDescriptorPool(numSwapChainImages);
	CreateDescriptorSets(descriptorSetLayout, numSwapChainImages);
}

// TODO: use push constants, more efficient
// this assumes every shader has the same type of uniform buffer for
// transformations
void GameObject::UpdateUniformBuffer(uint32_t imageIndex, const glm::mat4& viewMatrix,
									 VkExtent2D swapChainExtent) {
	UniformBufferObjectVert ubo = {};
	ubo.model = modelMatrix;
	ubo.view = viewMatrix;
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1; // flip Y -- opposite of opengl

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), uniformBuffersVert[imageIndex]->GetUniformBufferMemory(), 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(logicalDeviceManager->GetDevice(), uniformBuffersVert[imageIndex]->GetUniformBufferMemory());
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
		VkDescriptorBufferInfo bufferInfoVert = {};
		bufferInfoVert.buffer = uniformBuffersVert[i]->GetUniformBuffer();
		bufferInfoVert.offset = 0;
		bufferInfoVert.range = sizeof(UniformBufferObjectVert);
		
		VkDescriptorBufferInfo bufferInfoFrag = {};
		bufferInfoFrag.buffer = uniformBuffersVert[i]->GetUniformBuffer();
		bufferInfoFrag.offset = 0;
		bufferInfoFrag.range = sizeof(UniformBufferObjectLighting); // TODO: fix

		DescriptorSetFunctions::UpdateDescriptorSet(logicalDeviceManager->GetDevice(),
													materialType,
													descriptorSets[i],
													textureLoader->GetTextureImageView(),
													textureLoader->GetTextureImageSampler(),
													&bufferInfoVert,
													&bufferInfoFrag);
	}
}

void GameObject::CleanUpDescriptorPool() {
	if (descriptorPool != nullptr) {
		vkDestroyDescriptorPool(logicalDeviceManager->GetDevice(), descriptorPool, nullptr);
		descriptorPool = nullptr;
	}
}
