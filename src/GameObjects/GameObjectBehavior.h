#pragma once

#include <glm/glm.hpp>
#include "SceneManagement/Scene.h"

class MeshGameObject;

class GameObjectBehavior {
public:
	enum class BehaviorStatus : char { Normal = 0, Destroyed };

	GameObjectBehavior(Scene * scene)
		: modelMatrix(glm::mat4(1.0f)), scene(scene), gameObject(nullptr) {
	}

	GameObjectBehavior()
		: modelMatrix(glm::mat4(1.0f)), scene(nullptr), gameObject(nullptr) {
	}
	
	virtual ~GameObjectBehavior() {
	}

	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) = 0;
	
	void MultiplyByMatrix(glm::mat4 const& newMatrix) {
		this->modelMatrix *= newMatrix;
	}

	void SetModelMatrix(glm::mat4 const & modelMatrix) {
		this->modelMatrix = modelMatrix;
	}
	
	glm::mat4 const & GetModelMatrix() {
		return modelMatrix;
	}

	// Can be relative to world if base game object in scene graph,
	// or relative to parent.
	void SetRelativePosition(glm::vec3 const& relativePos) {
		modelMatrix[3][0] = relativePos[0];
		modelMatrix[3][1] = relativePos[1];
		modelMatrix[3][2] = relativePos[2];
	}

	glm::vec3 GetRelativePosition() const {
		return glm::vec3(modelMatrix[3][0], modelMatrix[3][1],
			modelMatrix[3][2]);
	}

	void SetScene(Scene* scene) {
		this->scene = scene;
	}

	void SetGameObject(MeshGameObject* gameObject) {
		this->gameObject = gameObject;
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
	// because a model matrix will mix up rotation and scale
	glm::mat4 modelMatrix;
	// we don't own this pointer; should be shared_ptr ideally?
	// the problem is that we want to de-allocate objects in a certain order
	// because an object relies on a vulkan logic device, the latter of which
	// should be deleted after. Need to enforce proper destruction order somehow
	// with shared pointers as opposed to using classical pointers.
	Scene * scene;
	MeshGameObject* gameObject;

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
		void * uboVoid, VkExtent2D const& swapChainExtent,
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
};
