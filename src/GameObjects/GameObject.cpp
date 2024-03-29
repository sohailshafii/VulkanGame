#include "GameObject.h"

GameObject::GameObject(std::string const& name) :
	gameObjectBehavior(nullptr),
	initializedInEngine(false),
	markedForDeletion(false),
	localTransform(1.0f),
	parentRelativeTransform(1.0f),
	localToWorld(1.0f),
	name(name) {
}

GameObject::GameObject(std::shared_ptr<GameObjectBehavior> behavior,
	std::string const & name) :
	gameObjectBehavior(behavior),
	initializedInEngine(false),
	markedForDeletion(false),
	localTransform(1.0f),
	parentRelativeTransform(1.0f),
	localToWorld(1.0f),
	name(name) {
}

GameObject::~GameObject() {
}

void GameObject::UpdateState(float time, float deltaTime) {
	if (gameObjectBehavior != nullptr) {
		auto behaviorStatus = gameObjectBehavior->UpdateSelf(time, deltaTime);
		if (behaviorStatus == GameObjectBehavior::BehaviorStatus::Destroyed) {
			markedForDeletion = true;
		}
	}
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


