#pragma once

#include "Resources/Material.h"
#include "Resources/Model.h"
#include "GameObjects/GameObjectBehavior.h"
#include <memory>
#include <vector>

/// <summary>
/// A standard game object that is by default, invisible.
/// </summary>
class GameObject {
public:
	GameObject(std::shared_ptr<GameObjectBehavior> behavior,
		std::shared_ptr<Model> const& model = nullptr,
		std::shared_ptr<Material> const& material = nullptr);
	virtual ~GameObject();

	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return material == nullptr ? DescriptorSetFunctions::MaterialType::Unspecified :
			material->GetMaterialType();
	}

	virtual bool IsInvisible() const {
		return true;
	}

	virtual std::shared_ptr<Model> GetModel() {
		return nullptr;
	}

	GameObjectBehavior* GetGameObjectBehavior() {
		return gameObjectBehavior.get();
	}

	glm::mat4 const& GetModelTransform() const {
		return gameObjectBehavior->GetModelMatrix();
	}

	glm::vec3 GetWorldPosition() const {
		return gameObjectBehavior->GetRelativePosition();
	}

	void SetModelTransform(const glm::mat4& model) {
		gameObjectBehavior->SetModelMatrix(model);
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

	virtual void UpdateState(float time, float deltaTime);
	virtual void UpdateVisualState(uint32_t imageIndex, const glm::mat4& viewMatrix,
		float time, float deltaTime,
		VkExtent2D swapChainExtent);

	virtual VkBuffer GetVertexBuffer() const {
		return VK_NULL_HANDLE;
	}

	virtual VkBuffer GetIndexBuffer() const {
		return VK_NULL_HANDLE;
	}

	virtual std::string GetVertexShaderName() const {
		return "";
	}

	virtual std::string GetFragmentShaderName() const {
		return "";
	}

	virtual VkDescriptorSet* GetDescriptorSetPtr(size_t swapChainIndex) {
		return nullptr;
	}

	virtual VkDescriptorSetLayout GetDescriptorSetLayout() const {
		return VK_NULL_HANDLE;
	}

	virtual VkPrimitiveTopology GetPrimitiveTopology() const {
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}

	virtual void InitAndCreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
		size_t numSwapChainImages) {
	}

	virtual void CreateDescriptorPoolAndSets(size_t numSwapChainImages) {
	}

	size_t GetNumChildGameObjects() const {
		return childGameObjects.size();
	}

	std::shared_ptr<GameObject> GetChildGameObject(size_t index) {
		return childGameObjects[index];
	}

	std::vector<std::shared_ptr<GameObject>> & GetChildren() {
		return childGameObjects;
	}

	void AddChildGameObject(std::shared_ptr<GameObject> const & newChild) {
		childGameObjects.push_back(newChild);
	}

	void RemoveChildGameObject(std::shared_ptr<GameObject> const& childToRm) {
		auto removeItr = std::remove_if(childGameObjects.begin(), childGameObjects.end(),
			[childToRm](std::shared_ptr<GameObject> const& currChild)
			{ return childToRm == currChild; });
		childGameObjects.erase(removeItr, childGameObjects.end());
	}

protected:
	std::vector<std::shared_ptr<GameObject>> childGameObjects;
	std::shared_ptr<GameObjectBehavior> gameObjectBehavior;

	std::shared_ptr<Model> objModel;
	std::shared_ptr<Material> material;

	bool initializedInEngine;
	bool markedForDeletion;

private:
	void UpdateChildrenStates(float time, float deltaTime);
	void UpdateChildrenVisualStates(uint32_t imageIndex, const glm::mat4& viewMatrix,
		float time, float deltaTime, VkExtent2D swapChainExtent);
};
