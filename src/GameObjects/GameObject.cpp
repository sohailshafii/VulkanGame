#include "GameObject.h"
#include "Resources/Model.h"
#include "Vertex.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Common.h"
#include "Resources/ImageTextureLoader.h"

#include <iostream>

GameObject::GameObject(std::shared_ptr<Model> const& model,
	std::shared_ptr<Material> const& material,
	std::unique_ptr<GameObjectBehavior> behavior,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) :
	objModel(model),
	material(material),
	gameObjectBehavior(std::move(behavior)),
	logicalDeviceManager(logicalDeviceManager),
	descriptorPool(nullptr),
	initializedInEngine(false),
	markedForDeletion(false) {
	SetupShaderNames();
	auto materialType = material->GetMaterialType();

	descriptorSetLayout = DescriptorSetFunctions::CreateDescriptorSetLayout(
		logicalDeviceManager->GetDevice(), materialType);

	if (materialType == DescriptorSetFunctions::MaterialType::UnlitColor) {
		CreateVertexBuffer(model->BuildAndReturnVertsPos(), gfxDeviceManager,
			commandPool);
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::UnlitTintedTextured) {
		CreateVertexBuffer(model->BuildAndReturnVertsPosColorTexCoord(), gfxDeviceManager,
							commandPool);
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::MotherShip) {
		CreateVertexBuffer(model->BuildAndReturnVertsPosColorTexCoord(), gfxDeviceManager,
			commandPool);
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::WavySurface) {
		CreateVertexBuffer(model->BuildAndReturnVertsPosNormalColorTexCoord(), gfxDeviceManager,
							commandPool);
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::BumpySurface) {
		CreateVertexBuffer(model->BuildAndReturnVertsPosNormalColorTexCoord(),
			gfxDeviceManager, commandPool);
	}
	
	CreateIndexBuffer(model->GetIndices(), gfxDeviceManager, commandPool);
}

template<typename VertexType>
void GameObject::CreateVertexBuffer(const std::vector<VertexType>& vertices,
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
	switch (material->GetMaterialType()) {
		case DescriptorSetFunctions::MaterialType::UnlitColor:
			vertexShaderName = "UnlitColorVert.spv";
			fragmentShaderName = "UnlitColorFrag.spv";
			break;
		case DescriptorSetFunctions::MaterialType::MotherShip:
			vertexShaderName = "MotherShip.spv";
			fragmentShaderName = "MotherShip.spv";
			break;
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
			vertexShaderName = "UnlitTintedTexturedVert.spv";
			fragmentShaderName = "UnlitTintedTexturedFrag.spv";
			break;
		case DescriptorSetFunctions::MaterialType::WavySurface:
			vertexShaderName = "WavySurfaceVert.spv";
			fragmentShaderName = "WavySurfaceFrag.spv";
			break;
		case DescriptorSetFunctions::MaterialType::BumpySurface:
			vertexShaderName = "BumpySurfaceVert.spv";
			fragmentShaderName = "BumpySurfaceFrag.spv";
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
	VkDeviceSize bufferSizeVert = GetMaterialUniformBufferSizeVert();
	VkDeviceSize bufferSizeLighting =
		material->GetMaterialType() == DescriptorSetFunctions::MaterialType::UnlitColor ?
		sizeof(UniformBufferFragUnlitColor) :
		sizeof(UniformBufferObjectLighting);
	
	for (size_t i = 0; i < numSwapChainImages; i++) {
		uniformBuffersVert.push_back(new GameObjectUniformBufferObj(logicalDeviceManager, gfxDeviceManager,
			(int)bufferSizeVert));
		uniformBuffersFrag.push_back(new GameObjectUniformBufferObj(logicalDeviceManager, gfxDeviceManager,
			(int)bufferSizeLighting));
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

void GameObject::InitAndCreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
							size_t numSwapChainImages) {
	CleanUpUniformBuffers();
	CreateUniformBuffers(gfxDeviceManager, numSwapChainImages);
}

void GameObject::CreateDescriptorPoolAndSets(size_t numSwapChainImages) {
	CleanUpDescriptorPool();
	CreateDescriptorPool(numSwapChainImages);
	CreateDescriptorSets(numSwapChainImages);
}

void GameObject::UpdateState(float time, float deltaTime) {
	auto behaviorStatus = gameObjectBehavior->UpdateSelf(time, deltaTime);
	if (behaviorStatus == GameObjectBehavior::BehaviorStatus::Destroyed) {
		markedForDeletion = true;
	}
}

// TODO: use push constants, more efficient
void GameObject::UpdateVisualState(uint32_t imageIndex,
								   const glm::mat4& viewMatrix,
								   float time, float deltaTime,
								   VkExtent2D swapChainExtent) {
	switch (material->GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
		case DescriptorSetFunctions::MaterialType::MotherShip:
			UpdateUniformBufferModelViewProj(imageIndex, viewMatrix,
											time, deltaTime, swapChainExtent);
			break;
		default:
			UpdateUniformBufferModelViewProjTime(imageIndex, viewMatrix,
												time, deltaTime, swapChainExtent);
			break;
	}
}

void GameObject::CreateDescriptorPool(size_t numSwapChainImages) {
	descriptorPool = DescriptorSetFunctions::CreateDescriptorPool(logicalDeviceManager->GetDevice(),
																  material->GetMaterialType(),
																  numSwapChainImages);
	std::cout << descriptorPool << " belongs to "
	<< material->GetMaterialType() << ", with layout " << std::endl;
}

void GameObject::CreateDescriptorSets(size_t numSwapChainImages) {
	std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages,
		descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
	allocInfo.pSetLayouts = layouts.data();
	
	std::cout << "about to allocate descriptor sets for pool "
	<< descriptorPool << ", with layout " << descriptorSetLayout << ", material: " << material->GetMaterialType() << std::endl;

	descriptorSets.resize(numSwapChainImages);
	if (vkAllocateDescriptorSets(logicalDeviceManager->GetDevice(), &allocInfo,
		descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}
	
	for (size_t i = 0; i < numSwapChainImages; ++i) {
		VkDescriptorBufferInfo bufferInfoVert = {};
		bufferInfoVert.buffer = uniformBuffersVert[i]->GetUniformBuffer();
		bufferInfoVert.offset = 0;
		bufferInfoVert.range = GetMaterialUniformBufferSizeVert();
		
		VkDescriptorBufferInfo bufferInfoFrag = {};
		bufferInfoFrag.buffer = uniformBuffersFrag[i]->GetUniformBuffer();
		bufferInfoFrag.offset = 0;
		bufferInfoFrag.range = uniformBuffersFrag[i]->GetBufferSize();

		DescriptorSetFunctions::UpdateDescriptorSet(logicalDeviceManager->GetDevice(),
			material->GetMaterialType(),
			descriptorSets[i],
			material->GetTextureLoader(),
			material->GetTintColor(),
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

VkDeviceSize GameObject::GetMaterialUniformBufferSizeVert()
{
	switch (material->GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
		case DescriptorSetFunctions::MaterialType::MotherShip:
			return sizeof(UniformBufferObjectModelViewProj);
			break;
		default:
			return sizeof(UniformBufferObjectModelViewProjTime);
			break;
	}
}

void GameObject::UpdateUniformBufferModelViewProj(uint32_t imageIndex,
												const glm::mat4& viewMatrix,
												float time,
												float deltaTime,
												VkExtent2D swapChainExtent)
{
	UniformBufferObjectModelViewProj ubo = {};
	ubo.model = gameObjectBehavior->GetModelMatrix();
	ubo.view = viewMatrix;
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo.proj[1][1] *= -1; // flip Y -- opposite of opengl

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), uniformBuffersVert[imageIndex]->GetUniformBufferMemory(), 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(logicalDeviceManager->GetDevice(), uniformBuffersVert[imageIndex]->GetUniformBufferMemory());
}

void GameObject::UpdateUniformBufferModelViewProjTime(uint32_t imageIndex,
													const glm::mat4& viewMatrix,
													float time, float deltaTime,
													VkExtent2D swapChainExtent)
{
	UniformBufferObjectModelViewProjTime ubo = {};
	ubo.model = gameObjectBehavior->GetModelMatrix();
	ubo.view = viewMatrix;
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo.proj[1][1] *= -1; // flip Y -- opposite of opengl
	ubo.time = time;

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), uniformBuffersVert[imageIndex]->GetUniformBufferMemory(), 0,
		sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(logicalDeviceManager->GetDevice(), uniformBuffersVert[imageIndex]->GetUniformBufferMemory());
}
