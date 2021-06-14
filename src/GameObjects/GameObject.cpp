#include "GameObject.h"
#include "Resources/Model.h"
#include "Vertex.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Common.h"
#include "Resources/TextureCreator.h"

#include <iostream>

GameObject::GameObject(std::shared_ptr<Model> const& model,
	std::shared_ptr<Material> const& material,
	std::shared_ptr<GameObjectBehavior> behavior,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) :
	objModel(model),
	material(material),
	gameObjectBehavior(behavior),
	logicalDeviceManager(logicalDeviceManager),
	descriptorPool(nullptr),
	initializedInEngine(false),
	markedForDeletion(false),
	vertexStagingBuffer(VK_NULL_HANDLE),
	vertexStagingBufferMemory(VK_NULL_HANDLE),
	vertexBuffer(VK_NULL_HANDLE),
	vertexBufferMemory(VK_NULL_HANDLE),
	indexStagingBuffer(VK_NULL_HANDLE),
	indexStagingBufferMemory(VK_NULL_HANDLE),
	indexBuffer(VK_NULL_HANDLE),
	indexBufferMemory(VK_NULL_HANDLE),
	commandPool(commandPool),
	gfxDeviceManager(gfxDeviceManager),
	vertUboData(nullptr), fragUboData(nullptr) {
	SetupShaderNames();

	gameObjectBehavior->SetGameObject(this);

	descriptorSetLayout = DescriptorSetFunctions::CreateDescriptorSetLayout(
		logicalDeviceManager->GetDevice(), GetMaterialType());

	CreateOrUpdateVertexBufferForMaterial(gfxDeviceManager,
		commandPool);

	CreateOrUpdateIndexBuffer(gfxDeviceManager, commandPool);
}

void GameObject::CreateOrUpdateVertexBufferForMaterial(GfxDeviceManager* gfxDeviceManager,
	VkCommandPool commandPool) {
	if (objModel == nullptr) {
		vertexBuffer = VK_NULL_HANDLE;
		vertexStagingBuffer = VK_NULL_HANDLE;
		return;
	}

	switch (GetMaterialType()) {
		case DescriptorSetFunctions::MaterialType::UnlitColor:
			CreateOrUpdateVertexBuffer(objModel->BuildAndReturnVertsPos(),
				gfxDeviceManager, commandPool);
			break;
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
			CreateOrUpdateVertexBuffer(objModel->BuildAndReturnVertsPosColorTexCoord(),
				gfxDeviceManager, commandPool);
			break;
		case DescriptorSetFunctions::MaterialType::MotherShip:
			CreateOrUpdateVertexBuffer(objModel->BuildAndReturnVertsPosColorTexCoord(),
				gfxDeviceManager, commandPool);
			break;
		case DescriptorSetFunctions::MaterialType::WavySurface:
			CreateOrUpdateVertexBuffer(objModel->BuildAndReturnVertsPosNormalColorTexCoord(),
				gfxDeviceManager, commandPool);
			break;
		case DescriptorSetFunctions::MaterialType::BumpySurface:
			CreateOrUpdateVertexBuffer(objModel->BuildAndReturnVertsPosNormalColorTexCoord(),
				gfxDeviceManager, commandPool);
			break;
		case DescriptorSetFunctions::MaterialType::Text:
			CreateOrUpdateVertexBuffer(objModel->BuildAndReturnVertsPosTex(),
				gfxDeviceManager, commandPool);
			break;
		default:
			vertexBuffer = VK_NULL_HANDLE;
			vertexStagingBuffer = VK_NULL_HANDLE;
			break;
	}
}

template<typename VertexType>
void GameObject::CreateOrUpdateVertexBuffer(std::vector<VertexType> const & vertices,
											GfxDeviceManager *gfxDeviceManager,
											VkCommandPool commandPool) {
	VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();
	if (bufferSize == 0) {
		return;
	}

	if (vertexStagingBuffer == VK_NULL_HANDLE) {
		Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexStagingBuffer, vertexStagingBufferMemory);
	}

	void *data;
	vkMapMemory(logicalDeviceManager->GetDevice(), vertexStagingBufferMemory, 0,
		bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDeviceManager->GetDevice(), vertexStagingBufferMemory);

	if (vertexBuffer == VK_NULL_HANDLE) {
		Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	}

	Common::CopyBuffer(logicalDeviceManager.get(), commandPool, vertexStagingBuffer,
		vertexBuffer, bufferSize);
}

GameObject::~GameObject() {
	if (vertUboData != nullptr) {
		delete vertUboData;
	}
	if (fragUboData != nullptr) {
		delete fragUboData;
	}

	if (vertexStagingBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(logicalDeviceManager->GetDevice(), vertexStagingBuffer, nullptr);
	}
	if (vertexStagingBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(logicalDeviceManager->GetDevice(), vertexStagingBufferMemory, nullptr);
	}
	if (vertexBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(logicalDeviceManager->GetDevice(), vertexBuffer, nullptr);
	}
	if (vertexBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(logicalDeviceManager->GetDevice(), vertexBufferMemory, nullptr);
	}
	
	if (indexStagingBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(logicalDeviceManager->GetDevice(), indexStagingBuffer, nullptr);
	}
	if (indexStagingBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(logicalDeviceManager->GetDevice(), indexStagingBufferMemory, nullptr);
	}
	if (indexBuffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(logicalDeviceManager->GetDevice(), indexBuffer, nullptr);
	}
	if (indexBufferMemory != VK_NULL_HANDLE) {
		vkFreeMemory(logicalDeviceManager->GetDevice(), indexBufferMemory, nullptr);
	}
	if (descriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(logicalDeviceManager->GetDevice(), descriptorSetLayout, nullptr);
	}

	CleanUpUniformBuffers();
	CleanUpDescriptorPool();
}

void GameObject::AllocateVBODataIfNecessary(size_t& uboSize,
	uint32_t imageIndex, const glm::mat4& viewMatrix, float time, float deltaTime,
	VkExtent2D swapChainExtent) {
	if (vertUboData != nullptr) {
		return;
	}
	if (IsInvisible()) {
		return;
	}
	vertUboData = gameObjectBehavior->CreateVertUBOData(uboSize, swapChainExtent,
		viewMatrix, time, deltaTime);
}

void GameObject::AllocateFBODataIfNecessary(size_t &uboSize) {
	if (fragUboData != nullptr) {
		return;
	}
	if (IsInvisible()) {
		return;
	}
	fragUboData = gameObjectBehavior->CreateFragUBOData(uboSize);
}

void GameObject::SetupShaderNames() {
	switch (GetMaterialType()) {
		case DescriptorSetFunctions::MaterialType::UnlitColor:
			vertexShaderName = "UnlitColorVert.spv";
			fragmentShaderName = "UnlitColorFrag.spv";
			break;
		case DescriptorSetFunctions::MaterialType::MotherShip:
			vertexShaderName = "MotherShipVert.spv";
			fragmentShaderName = "MotherShipFrag.spv";
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
		case DescriptorSetFunctions::MaterialType::Text:
			vertexShaderName = "TextShaderVert.spv";
			fragmentShaderName = "TextShaderFrag.spv";
			break;
		default:
			vertexShaderName = "";
			fragmentShaderName = "";
			break;
	}
}

void GameObject::CreateOrUpdateIndexBuffer(GfxDeviceManager *gfxDeviceManager,
											VkCommandPool commandPool) {
	if (objModel == nullptr) {
		return;
	}
	std::vector<uint32_t> const& indices = objModel->GetIndices();

	VkDeviceSize bufferSize = sizeof(indices[0])*indices.size();

	if (indexStagingBuffer == VK_NULL_HANDLE) {
		Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager,
			bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexStagingBuffer, indexStagingBufferMemory);
	}

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), indexStagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(logicalDeviceManager->GetDevice(), indexStagingBufferMemory);

	if (indexBuffer == VK_NULL_HANDLE) {
		Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
	}

	Common::CopyBuffer(logicalDeviceManager.get(), commandPool, indexStagingBuffer,
		indexBuffer, bufferSize);
}

void GameObject::CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager, size_t numSwapChainImages) {
	if (IsInvisible()) {
		return;
	}
	VkDeviceSize bufferSizeVert = GetMaterialUniformBufferSizeVert();
	VkDeviceSize bufferSizeFrag = GetMaterialUniformBufferSizeFrag();
	
	for (size_t i = 0; i < numSwapChainImages; i++) {
		uniformBuffersVert.push_back(new GameObjectUniformBufferObj(logicalDeviceManager, gfxDeviceManager,
			(int)bufferSizeVert));
		uniformBuffersFrag.push_back(new GameObjectUniformBufferObj(logicalDeviceManager, gfxDeviceManager,
			(int)bufferSizeFrag));
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

void GameObject::UpdateVisualState(uint32_t imageIndex,
								   const glm::mat4& viewMatrix,
								   float time, float deltaTime,
								   VkExtent2D swapChainExtent) {
	if (IsInvisible()) {
		return;
	}

	
	AllocateVBODataIfNecessary(vertUboSize, imageIndex, viewMatrix,
		time, deltaTime, swapChainExtent);
	gameObjectBehavior->UpdateVertUBOData(vertUboData,
		swapChainExtent, viewMatrix, time, deltaTime);
	if (vertUboData != nullptr) {
		void* data;
		vkMapMemory(logicalDeviceManager->GetDevice(),
			uniformBuffersVert[imageIndex]->GetUniformBufferMemory(), 0,
			vertUboSize, 0, &data);
		if (vertUboSize != uniformBuffersVert[imageIndex]->GetBufferSize()) {
			int breakVar;
			breakVar = 1;
		}
		memcpy(data, vertUboData, vertUboSize);
		vkUnmapMemory(logicalDeviceManager->GetDevice(),
			uniformBuffersVert[imageIndex]->GetUniformBufferMemory());
	}

	AllocateFBODataIfNecessary(fragUboSize);
	gameObjectBehavior->UpdateFragUBOData(fragUboData);
	if (fragUboData != nullptr) {
		void* data;
		vkMapMemory(logicalDeviceManager->GetDevice(),
			uniformBuffersFrag[imageIndex]->GetUniformBufferMemory(), 0,
			fragUboSize, 0, &data);
		memcpy(data, fragUboData, fragUboSize);
		vkUnmapMemory(logicalDeviceManager->GetDevice(),
			uniformBuffersFrag[imageIndex]->GetUniformBufferMemory());
	}
}

void GameObject::UpdateVertexBufferWithLatestModelVerts() {
	CreateOrUpdateVertexBufferForMaterial(gfxDeviceManager,
		commandPool);
}

void GameObject::CreateDescriptorPool(size_t numSwapChainImages) {
	if (IsInvisible()) {
		return;
	}
	descriptorPool = DescriptorSetFunctions::CreateDescriptorPool(logicalDeviceManager->GetDevice(),
		GetMaterialType(), numSwapChainImages);
	std::cout << descriptorPool << " belongs to "
		<< static_cast<int>(GetMaterialType()) << ", with layout " << std::endl;
}

void GameObject::CreateDescriptorSets(size_t numSwapChainImages) {
	if (IsInvisible()) {
		return;
	}

	std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages,
		descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
	allocInfo.pSetLayouts = layouts.data();
	
	std::cout << "about to allocate descriptor sets for pool "
	<< descriptorPool << ", with layout " << descriptorSetLayout << ", material: " << 
		static_cast<int>(GetMaterialType()) << std::endl;

	descriptorSets.resize(numSwapChainImages);
	if (vkAllocateDescriptorSets(logicalDeviceManager->GetDevice(), &allocInfo,
		descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}
	
	for (size_t i = 0; i < numSwapChainImages; ++i) {
		VkDescriptorBufferInfo bufferInfoVert = {};
		bufferInfoVert.buffer = uniformBuffersVert[i]->GetUniformBuffer();
		bufferInfoVert.offset = 0;
		bufferInfoVert.range = uniformBuffersVert[i]->GetBufferSize();
		
		VkDescriptorBufferInfo bufferInfoFrag = {};
		bufferInfoFrag.buffer = uniformBuffersFrag[i]->GetUniformBuffer();
		bufferInfoFrag.offset = 0;
		bufferInfoFrag.range = uniformBuffersFrag[i]->GetBufferSize();

		DescriptorSetFunctions::UpdateDescriptorSet(logicalDeviceManager->GetDevice(),
			material,
			descriptorSets[i],
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
		case DescriptorSetFunctions::MaterialType::Text:
			return sizeof(UniformBufferObjectModelViewProj);
			break;
		case DescriptorSetFunctions::MaterialType::MotherShip:
			return sizeof(UniformBufferObjectModelViewProjRipple);
			break;
		case DescriptorSetFunctions::MaterialType::BumpySurface:
		case DescriptorSetFunctions::MaterialType::WavySurface:
			return sizeof(UniformBufferObjectModelViewProjTime);
			break;
		default:
			return 0;
	}
}

VkDeviceSize GameObject::GetMaterialUniformBufferSizeFrag() {
	VkDeviceSize bufferSizeFrag = sizeof(UniformBufferObjectLighting);
	auto materialType = material->GetMaterialType();
	if (materialType == DescriptorSetFunctions::MaterialType::UnlitColor ||
		materialType == DescriptorSetFunctions::MaterialType::Text) {
		bufferSizeFrag = sizeof(UniformBufferUnlitColor);
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::Unspecified) {
		bufferSizeFrag = 0;
	}

	return bufferSizeFrag;
}
