#include "GameObject.h"

GameObject::GameObject(std::shared_ptr<GameObjectBehavior> behavior,
	std::shared_ptr<Model> const& model,
	std::shared_ptr<Material> const& material) :
	gameObjectBehavior(behavior),
	objModel(model),
	material(material),
	initializedInEngine(false),
	markedForDeletion(false) {
}

GameObject::~GameObject() {
	
}

void GameObject::UpdateState(float time, float deltaTime) {
	UpdateChildrenStates(time, deltaTime);
}

void GameObject::UpdateVisualState(uint32_t imageIndex, const glm::mat4& viewMatrix,
	float time, float deltaTime, VkExtent2D swapChainExtent) {
	UpdateChildrenVisualStates(imageIndex, viewMatrix, time,
		deltaTime, swapChainExtent);
}

void GameObject::UpdateChildrenStates(float time, float deltaTime) {
	for (auto& gameObject : childGameObjects) {
		gameObject->UpdateState(time, deltaTime);
	}
}

void GameObject::UpdateChildrenVisualStates(uint32_t imageIndex,
	const glm::mat4& viewMatrix, float time, float deltaTime, VkExtent2D swapChainExtent) {
	for (auto& gameObject : childGameObjects) {
		gameObject->UpdateVisualState(imageIndex, viewMatrix, time,
			deltaTime, swapChainExtent);
	}
}
