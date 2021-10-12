#pragma once

#include <memory>
#include <vector>
#include <string>
#include "vulkan/vulkan.h"
#include <glm/glm.hpp>
#include "DescriptorSetFunctions.h"
#include "GameObjects/GameObject.h"
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
class MeshGameObject : public GameObject {
public:
	MeshGameObject(std::shared_ptr<GameObjectBehavior> behavior,
		GfxDeviceManager *gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool,
		std::shared_ptr<Model> const& model = nullptr,
		std::shared_ptr<Material> const& material = nullptr,
		std::string const & name = "");

	MeshGameObject(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool,
		std::string const& name = "");
	
	~MeshGameObject();

	virtual DescriptorSetFunctions::MaterialType GetMaterialType() const override {
		return material == nullptr ? DescriptorSetFunctions::MaterialType::Unspecified :
			material->GetMaterialType();
	}

	void InitializeMeshState();

	virtual bool IsInvisible() const override {
		return GetMaterialType() ==
			DescriptorSetFunctions::MaterialType::Unspecified ||
			objModel == nullptr;
	}
	
	virtual std::shared_ptr<Model> GetModel() override {
		return objModel;
	}
	
	virtual std::string GetVertexShaderName() const override {
		return vertexShaderName;
	}
	
	virtual std::string GetFragmentShaderName() const override {
		return fragmentShaderName;
	}
	
	virtual VkBuffer GetVertexBuffer() const override {
		return vertexBuffer;
	}
	
	virtual VkBuffer GetIndexBuffer() const override {
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
	
	virtual VkDescriptorSet* GetDescriptorSetPtr(size_t swapChainIndex) override {
		return &descriptorSets[swapChainIndex];
	}
	
	virtual VkDescriptorSetLayout GetDescriptorSetLayout() const override {
		return descriptorSetLayout;
	}

	virtual VkPrimitiveTopology GetPrimitiveTopology() const override {
		return objModel->GetTopologyType() == Model::TopologyType::TriangleList
			? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST :
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	}
	
	virtual void InitAndCreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
											size_t numSwapChainImages) override;
	
	virtual void CreateDescriptorPoolAndSets(size_t numSwapChainImages) override;
	
	virtual void UpdateVisualState(uint32_t imageIndex, const glm::mat4& viewMatrix,
									float time, float deltaTime,
									VkExtent2D swapChainExtent) override;
	
	virtual void UpdateVertexBufferWithLatestModelVerts() override;
	
protected:
	std::shared_ptr<Model> objModel;
	std::shared_ptr<Material> material;

private:
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

	void* vertUboData;
	size_t vertUboSize;
	void* fragUboData;
	size_t fragUboSize;
	
	
	void AllocateFragUBODataIfNecessary(size_t& uboSize);

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

	void AllocateVertexUBODataIfNecessary(size_t& uboSize, uint32_t imageIndex,
		const glm::mat4& viewMatrix, float time, float deltaTime,
		VkExtent2D swapChainExtent);
	void* CreateVertUBOData(size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix, float time, float deltaTime);
	void UpdateVertUBOData(void* vboData, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix, float time, float deltaTime);

	void* CreateFragUBOData(size_t& uboSize);
	void UpdateFragUBOData(void* vboData);

	// these can be overwritten by inheritors
	// assuming specific behaviors want to write to uniform buffers differently
	virtual void* CreateUniformBufferModelViewProj(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime);
	virtual void* CreateUniformBufferModelViewProjRipple(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime);
	virtual void* CreateUniformBufferModelViewProjTime(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime);

	virtual void UpdateUniformBufferModelViewProj(
		void* uboVoid, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime);
	virtual void UpdateUniformBufferModelViewProjRipple(
		void* uboVoid, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime);
	virtual void UpdateUniformBufferModelViewProjTime(
		void* uboVoid, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime);

	virtual void* CreateFragUniformBufferColor(size_t& uboSize);
	virtual void UpdateFragUniformBufferColor(void* uboVoid);
};
