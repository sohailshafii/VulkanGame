#include "GameObject.h"

GameObject::GameObject() :
	gameObjectBehavior(nullptr),
	initializedInEngine(false),
	markedForDeletion(false),
	localTransform(1.0f),
	parentRelativeTransform(1.0f),
	localToWorld(1.0f) {
}

GameObject::GameObject(std::shared_ptr<GameObjectBehavior> behavior) :
	gameObjectBehavior(behavior),
	initializedInEngine(false),
	markedForDeletion(false),
	localTransform(1.0f),
	parentRelativeTransform(1.0f),
	localToWorld(1.0f) {
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


