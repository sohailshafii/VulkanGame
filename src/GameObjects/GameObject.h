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
	// TODO: material class
	// TODO: pass in unique ptr to game object behavior here
	GameObject(std::shared_ptr<Model> const& model,
		std::shared_ptr<Material> const& material,
		//std::unique_ptr<GameObjectBehavior> behavior,
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
	
	glm::mat4 GetModelTransform() const {
		return modelMatrix;
	}
	
	void SetModelTransform(const glm::mat4& model) {
		this->modelMatrix = model;
	}
	
	VkBuffer GetVertexBuffer() const {
		return vertexBuffer;
	}
	
	VkBuffer GetIndexBuffer() const {
		return indexBuffer;
	}
	
	VkBuffer GetUniformBufferVert(size_t index) const {
		return uniformBuffersVert[index]->GetUniformBuffer();
	}
	
	VkBuffer GetUniformBufferFrag(size_t index) const {
		return uniformBuffersFrag[index]->GetUniformBuffer();
	}
	
	VkDeviceMemory GetUniformBufferMemoryVert(size_t index) {
		return uniformBuffersVert[index]->GetUniformBufferMemory();
	}
	
	VkDeviceMemory GetUniformBufferMemoryFrag(size_t index) {
		return uniformBuffersFrag[index]->GetUniformBufferMemory();
	}
	
	VkDescriptorSet* GetDescriptorSetPtr(size_t index) {
		return &descriptorSets[index];
	}
	
	VkDescriptorSetLayout GetDescriptorSetLayout() const {
		return descriptorSetLayout;
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return material->GetMaterialType();
	}

	VkPrimitiveTopology GetPrimitiveTopology() const {
		return objModel->GetTopologyType() == Model::TopologyType::TriangleList
			? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST :
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	}
	
	void CreateCommandBuffers(GfxDeviceManager* gfxDeviceManager,
								size_t numSwapChainImages);
	
	void CreateDescriptorPoolAndSets(size_t numSwapChainImages);
	
	void UpdateUniformBuffer(uint32_t imageIndex, const glm::mat4& viewMatrix,
							float time, VkExtent2D swapChainExtent);
	
private:
	std::shared_ptr<Model> objModel;
	std::shared_ptr<Material> material;
	std::unique_ptr<GameObjectBehavior> gameObjectBehavior;
	
	std::string vertexShaderName;
	std::string fragmentShaderName;
	
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	
	glm::mat4 modelMatrix;
	
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	
	std::vector<GameObjectUniformBufferObj*> uniformBuffersVert, uniformBuffersFrag;
	
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	
	void SetupShaderNames();
	
	template<typename VertexType>
	void CreateVertexBuffer(const std::vector<VertexType>& vertices,
							GfxDeviceManager *gfxDeviceManager,
							VkCommandPool commandPool);
	
	void CreateIndexBuffer(const std::vector<uint32_t>& indices,
						   GfxDeviceManager *gfxDeviceManager,
						   VkCommandPool commandPool);
	
	void CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
							  size_t numSwapChainImages);
	void CleanUpUniformBuffers();
	
	void CreateDescriptorPool(size_t numSwapChainImages);
	void CreateDescriptorSets(size_t numSwapChainImages);
	void CleanUpDescriptorPool();
	
	VkDeviceSize GetMaterialUniformBufferSizeVert();
	void UpdateUniformBufferUnlitTintedTextured(uint32_t imageIndex,
		const glm::mat4& viewMatrix, float time, VkExtent2D swapChainExtent);
	void UpdateUniformBufferGerstner(uint32_t imageIndex,
		const glm::mat4& viewMatrix, float time, VkExtent2D swapChainExtent);
};