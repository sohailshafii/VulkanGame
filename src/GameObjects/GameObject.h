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

	void AffectByTransform(glm::mat4 const & otherTransform) {
		this->localTransform = this->localTransform * otherTransform;
		this->localToWorld = this->parentRelativeTransform *
			this->localTransform;
	}

	void SetLocalPosition(glm::vec3 const& pos) {
		localTransform[3][0] = pos[0];
		localTransform[3][1] = pos[1];
		localTransform[3][2] = pos[2];
		this->localToWorld = this->parentRelativeTransform *
			this->localTransform;
	}

	glm::vec3 GetLocalPosition() const {
		return glm::vec3(localTransform[3][0], localTransform[3][1],
			localTransform[3][2]);
	}

	void SetLocalTransform(const glm::mat4& model) {
		this->localTransform = model;
		this->localToWorld = this->parentRelativeTransform * this->localTransform;
		for (auto& gameObject : childGameObjects) {
			gameObject->SetParentRelativeTransform(localToWorld);
		}
	}

	glm::mat4 GetLocalTransform() const {
		return localToWorld;
	}

	void SetParentRelativeTransform(const glm::mat4& model) {
		this->parentRelativeTransform = model;
		this->localToWorld = this->parentRelativeTransform * this->localTransform;
		if (childGameObjects.size() > 0) {
			for (auto& gameObject : childGameObjects) {
				gameObject->SetParentRelativeTransform(localToWorld);
			}
		}
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

	// provide specific information to callers about UBO.
	// depends on material used
	void* CreateVertUBOData(size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix, float time, float deltaTime);
	void UpdateVertUBOData(void* vboData, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix, float time, float deltaTime);

	virtual void* CreateFragUBOData(size_t& uboSize);
	void UpdateFragUBOData(void* vboData);

protected:
	std::vector<std::shared_ptr<GameObject>> childGameObjects;
	std::shared_ptr<GameObjectBehavior> gameObjectBehavior;

	bool initializedInEngine;
	bool markedForDeletion;

	glm::mat4 localTransform;
	glm::mat4 parentRelativeTransform;
	glm::mat4 localToWorld;

	// these can be overwritten by inheritors
	// assuming specific behaviors want to write to UBOs differently
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

	virtual void* CreateFBOUniformBufferColor(size_t& uboSize);
	virtual void UpdateFBOUniformBufferColor(void* uboVoid);

private:
	void UpdateChildrenStates(float time, float deltaTime);
	void UpdateChildrenVisualStates(uint32_t imageIndex, const glm::mat4& viewMatrix,
		float time, float deltaTime, VkExtent2D swapChainExtent);
};
