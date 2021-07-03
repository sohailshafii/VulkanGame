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
}

void GameObject::UpdateVisualState(uint32_t imageIndex, const glm::mat4& viewMatrix,
	float time, float deltaTime,
	VkExtent2D swapChainExtent) {
}
