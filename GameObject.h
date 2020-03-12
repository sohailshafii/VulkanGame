#pragma once

#include <memory>
#include <vector>
#include <string>
#include "vulkan/vulkan.h"
#include <glm/glm.hpp>
#include "DescriptorSetFunctions.h"
#include "GameObjectUniformBufferObj.h"

class Model;
struct Vertex;
class GfxDeviceManager;
class LogicalDeviceManager;
class ImageTextureLoader;

// game object with its own transform
// class will grow over time to include other relevant meta data
class GameObject {
public:
	// TODO: verify usage of shared_ptr -- should it be referenced here?
	// TODO: material class
	GameObject(std::shared_ptr<Model> model,
			   GfxDeviceManager *gfxDeviceManager,
			   std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
			   std::shared_ptr<ImageTextureLoader> textureLoader,
			   const std::string& vertexShaderName,
			   const std::string& fragmentShaderName,
			   VkCommandPool commandPool,
			   DescriptorSetFunctions::MaterialType materialType);
	
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
	
	void CreateCommandBuffers(GfxDeviceManager* gfxDeviceManager,
								size_t numSwapChainImages);
	
	void CreateDescriptorPoolAndSets(size_t numSwapChainImages);
	
	void UpdateUniformBuffer(uint32_t imageIndex, const glm::mat4& viewMatrix,
							 VkExtent2D swapChainExtent);
	
private:
	std::shared_ptr<Model> objModel;
	std::shared_ptr<ImageTextureLoader> textureLoader;
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
	DescriptorSetFunctions::MaterialType materialType;
	VkDescriptorSetLayout descriptorSetLayout;
	std::vector<VkDescriptorSet> descriptorSets;
	
	void CreateVertexBuffer(const std::vector<Vertex>& vertices,
							GfxDeviceManager *gfxDeviceManager,
							VkCommandPool commandPool);
	void CreateIndexBuffer(const std::vector<uint32_t>& indices,
						   GfxDeviceManager *gfxDeviceManager,
						   VkCommandPool commandPool);
	
	void CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
							  size_t numSwapChainImages);
	void CleanUpUniformBuffers();
	
	void CreateDescriptorPool(size_t numSwapChainImages);
	void CreateDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
							  size_t numSwapChainImages);
	void CleanUpDescriptorPool();
	
};
