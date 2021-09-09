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
	GameObject();

	GameObject(std::shared_ptr<GameObjectBehavior> behavior);

	virtual ~GameObject();

	virtual DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return DescriptorSetFunctions::MaterialType::Unspecified;
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

	glm::vec3 GetWorldPosition() const {
		return glm::vec3(localToWorld[3][0], localToWorld[3][1],
			localToWorld[3][2]);
	}

	glm::mat4 GetLocalToWorld() const {
		return localToWorld;
	}

	void AffectLocalTransform(glm::mat4 const & otherTransform) {
		this->localTransform = this->localTransform * otherTransform;
		this->localToWorld = this->parentRelativeTransform *
			this->localTransform;

		UpdateChildTransforms();
	}

	void SetLocalPosition(glm::vec3 const & pos) {
		localTransform[3][0] = pos[0];
		localTransform[3][1] = pos[1];
		localTransform[3][2] = pos[2];
		this->localToWorld = this->parentRelativeTransform *
			this->localTransform;

		UpdateChildTransforms();
	}

	glm::vec3 GetLocalPosition() const {
		return glm::vec3(localTransform[3][0], localTransform[3][1],
			localTransform[3][2]);
	}

	void SetLocalTransform(glm::mat4 const & model) {
		this->localTransform = model;
		this->localToWorld = this->parentRelativeTransform * this->localTransform;

		UpdateChildTransforms();
	}

	glm::mat4 GetLocalTransform() const {
		return localToWorld;
	}

	void SetParentRelativeTransform(glm::mat4 const & model) {
		this->parentRelativeTransform = model;
		this->localToWorld = this->parentRelativeTransform * this->localTransform;

		UpdateChildTransforms();
	}

	void SetWorldTransform(glm::mat4 const& matrix) {
		// affect local transform in such a way that world transform is affected
		this->localToWorld = matrix;
		this->localTransform = glm::inverse(this->parentRelativeTransform) * this->localToWorld;
		UpdateChildTransforms();
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

	// TODO: move these to visual class for game object
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
		newChild->SetParentRelativeTransform(localToWorld);
	}

	void RemoveChildGameObject(std::shared_ptr<GameObject> const& childToRm) {
		auto removeItr = std::remove_if(childGameObjects.begin(), childGameObjects.end(),
			[childToRm](std::shared_ptr<GameObject> const& currChild)
			{ return childToRm == currChild; });
		childGameObjects.erase(removeItr, childGameObjects.end());
	}

	virtual void UpdateVertexBufferWithLatestModelVerts() {
		// empty by default
	}

protected:
	std::vector<std::shared_ptr<GameObject>> childGameObjects;
	std::shared_ptr<GameObjectBehavior> gameObjectBehavior;

	bool initializedInEngine;
	bool markedForDeletion;

	glm::mat4 localTransform;
	glm::mat4 parentRelativeTransform;
	glm::mat4 localToWorld;

	void UpdateChildTransforms() {
		if (childGameObjects.size() > 0) {
			for (auto& gameObject : childGameObjects) {
				gameObject->SetParentRelativeTransform(localToWorld);
			}
		}
	}

private:
	void UpdateChildrenStates(float time, float deltaTime);
	void UpdateChildrenVisualStates(uint32_t imageIndex, const glm::mat4& viewMatrix,
		float time, float deltaTime, VkExtent2D swapChainExtent);
};
