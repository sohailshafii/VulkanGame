#pragma once

#include <memory>
#include <vector>
#include <string>
#include "vulkan/vulkan.h"
#include <glm/glm.hpp>
#include "DescriptorSetFunctions.h"
#include "GameObjects/GameObjectUniformBufferObj.h"
#include "GameObjects/GameObjectBehavior.h"
#include "Resources/Material.h"
#include "Resources/Model.h"

struct VertexPosColorTexCoord;
class GfxDeviceManager;
class LogicalDeviceManager;
class ImageTextureLoader;

enum GameObjectType
{
	Player = 0, Stationary, AI
};

// game object with its own transform
// class will grow over time to include other relevant meta data
// Game Object has an associated behavior class, which
// depends on its type -- stationary, AI (enemy), or player
class GameObject {
public:
	// TODO: make material and model option. in case object is invisible
	GameObject(std::shared_ptr<Model> const& model,
		std::shared_ptr<Material> const& material,
		std::shared_ptr<GameObjectBehavior> behavior,
		GfxDeviceManager *gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);
	
	~GameObject();
	
	std::shared_ptr<Model> GetModel() {
		return objModel;
	}
	
	std::string GetVertexShaderName() const {
		return vertexShaderName;
	}
	
	std::string GetFragmentShaderName() const {
		return fragmentShaderName;
	}
	
	glm::mat4 const & GetModelTransform() const {
		return gameObjectBehavior->GetModelMatrix();
	}

	glm::vec3 GetWorldPosition() const {
		return gameObjectBehavior->GetWorldPosition();
	}
	
	void SetModelTransform(const glm::mat4& model) {
		gameObjectBehavior->SetModelMatrix(model);
	}

	GameObjectBehavior* GetGameObjectBehavior() {
		return gameObjectBehavior.get();
	}
	
	VkBuffer GetVertexBuffer() const {
		return vertexBuffer;
	}
	
	VkBuffer GetIndexBuffer() const {
		return indexBuffer;
	}
	
	VkBuffer GetUniformBufferVert(size_t swapChainIndex) const {
		return uniformBuffersVert[swapChainIndex]->GetUniformBuffer();
	}
	
	VkBuffer GetUniformBufferFrag(size_t swapChainIndex) const {
		return uniformBuffersFrag[swapChainIndex]->GetUniformBuffer();
	}
	
	VkDeviceMemory GetUniformBufferMemoryVert(size_t swapChainIndex) {
		return uniformBuffersVert[swapChainIndex]->GetUniformBufferMemory();
	}
	
	VkDeviceMemory GetUniformBufferMemoryFrag(size_t swapChainIndex) {
		return uniformBuffersFrag[swapChainIndex]->GetUniformBufferMemory();
	}
	
	VkDescriptorSet* GetDescriptorSetPtr(size_t swapChainIndex) {
		return &descriptorSets[swapChainIndex];
	}
	
	VkDescriptorSetLayout GetDescriptorSetLayout() const {
		return descriptorSetLayout;
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return material == nullptr ? DescriptorSetFunctions::MaterialType::Unspecified :
			material->GetMaterialType();
	}

	bool IsInvisible() const {
		return GetMaterialType() ==
			DescriptorSetFunctions::MaterialType::Unspecified ||
			objModel == nullptr;
	}

	VkPrimitiveTopology GetPrimitiveTopology() const {
		return objModel->GetTopologyType() == Model::TopologyType::TriangleList
			? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST :
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	}

	bool GetInitializedInEngine() const {
		return initializedInEngine;
	}

	void SetInitializedInEngine(bool value) {
		initializedInEngine = value;
	}

	bool GetMarkedForDeletion() const {
		return markedForDeletion;
	}

	void SetMarkedForDeletionInScene(bool value) {
		markedForDeletion = value;
	}
	
	void InitAndCreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
								size_t numSwapChainImages);
	
	void CreateDescriptorPoolAndSets(size_t numSwapChainImages);
	
	void UpdateState(float time, float deltaTime);
	void UpdateVisualState(uint32_t imageIndex, const glm::mat4& viewMatrix,
						   float time, float deltaTime,
						   VkExtent2D swapChainExtent);
	
	void UpdateVertexBufferWithLatestModelVerts();
	
private:
	std::shared_ptr<Model> objModel;
	std::shared_ptr<Material> material;
	std::shared_ptr<GameObjectBehavior> gameObjectBehavior;
	
	std::string vertexShaderName;
	std::string fragmentShaderName;
	
	VkBuffer vertexStagingBuffer;
	VkDeviceMemory vertexStagingBufferMemory;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexStagingBuffer;
	VkDeviceMemory indexStagingBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	
	std::vector<GameObjectUniformBufferObj*> uniformBuffersVert, uniformBuffersFrag;
	
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;

	VkCommandPool commandPool;
	GfxDeviceManager* gfxDeviceManager; // TODO UGH, use smart pointer

	bool initializedInEngine;
	bool markedForDeletion;

	void* vertUboData;
	size_t vertUboSize;
	void* fragUboData;
	size_t fragUboSize;
	
	void AllocateVBODataIfNecessary(size_t& uboSize, uint32_t imageIndex,
		const glm::mat4& viewMatrix, float time, float deltaTime,
		VkExtent2D swapChainExtent);
	void AllocateFBODataIfNecessary(size_t& uboSize);

	void SetupShaderNames();
	
	void CreateOrUpdateVertexBufferForMaterial(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);

	template<typename VertexType>
	void CreateOrUpdateVertexBuffer(std::vector<VertexType> const & vertices,
									GfxDeviceManager *gfxDeviceManager,
									VkCommandPool commandPool);
	void CreateOrUpdateIndexBuffer(GfxDeviceManager *gfxDeviceManager,
									VkCommandPool commandPool);
	
	void CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
							  size_t numSwapChainImages);
	void CleanUpUniformBuffers();
	
	void CreateDescriptorPool(size_t numSwapChainImages);
	void CreateDescriptorSets(size_t numSwapChainImages);
	void CleanUpDescriptorPool();
	
	VkDeviceSize GetMaterialUniformBufferSizeVert();
	VkDeviceSize GetMaterialUniformBufferSizeFrag();
};
