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

void* GameObject::CreateVertUBOData(size_t& uboSize,
	VkExtent2D const& swapChainExtent, const glm::mat4& viewMatrix,
	float time, float deltaTime) {
	switch (GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
		case DescriptorSetFunctions::MaterialType::Text:
			return CreateUniformBufferModelViewProj(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
		case DescriptorSetFunctions::MaterialType::MotherShip:
			return CreateUniformBufferModelViewProjRipple(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
		case DescriptorSetFunctions::MaterialType::WavySurface:
		case DescriptorSetFunctions::MaterialType::BumpySurface:
			return CreateUniformBufferModelViewProjTime(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
		default:
			return nullptr;
	}
}

void GameObject::UpdateVertUBOData(void* vboData,
	VkExtent2D const& swapChainExtent, const glm::mat4& viewMatrix,
	float time, float deltaTime) {
	switch (GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
		case DescriptorSetFunctions::MaterialType::Text:
			return UpdateUniformBufferModelViewProj(vboData,
				swapChainExtent, viewMatrix, time, deltaTime);
		case DescriptorSetFunctions::MaterialType::MotherShip:
			return UpdateUniformBufferModelViewProjRipple(vboData,
				swapChainExtent, viewMatrix, time, deltaTime);
		case DescriptorSetFunctions::MaterialType::WavySurface:
		case DescriptorSetFunctions::MaterialType::BumpySurface:
			return UpdateUniformBufferModelViewProjTime(vboData,
				swapChainExtent, viewMatrix, time, deltaTime);
	}
}

void* GameObject::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	uboSize = sizeof(*ubo);

	switch (GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::Text:
			return CreateFBOUniformBufferColor(uboSize);
		default:
			return nullptr;
	}
	return ubo;
}

void GameObject::UpdateFragUBOData(void* vboData) {
	switch (GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::Text:
			return UpdateFBOUniformBufferColor(vboData);
	}
}

void* GameObject::CreateUniformBufferModelViewProj(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProj* ubo =
		new UniformBufferObjectModelViewProj();
	ubo->model = localToWorld;
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);

	uboSize = sizeof(UniformBufferObjectModelViewProj);
	return ubo;
}

void* GameObject::CreateUniformBufferModelViewProjRipple(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		new UniformBufferObjectModelViewProjRipple();
	ubo->model = localToWorld;
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);

	uboSize = sizeof(*ubo);
	return ubo;
}

void* GameObject::CreateUniformBufferModelViewProjTime(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjTime* ubo =
		new UniformBufferObjectModelViewProjTime();
	ubo->model = localToWorld;
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
	ubo->time = time;

	uboSize = sizeof(*ubo);
	return ubo;
}

void GameObject::UpdateUniformBufferModelViewProj(
	void* uboVoid, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProj* ubo =
		(UniformBufferObjectModelViewProj*)uboVoid;
	ubo->model = localToWorld;
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
}

void GameObject::UpdateUniformBufferModelViewProjRipple(
	void* uboVoid, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		(UniformBufferObjectModelViewProjRipple*)uboVoid;
	ubo->model = localToWorld;
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
}

void GameObject::UpdateUniformBufferModelViewProjTime(
	void* uboVoid, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjTime* ubo =
		(UniformBufferObjectModelViewProjTime*)uboVoid;
	ubo->model = localToWorld;
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
	ubo->time = time;
}

void* GameObject::CreateFBOUniformBufferColor(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = glm::vec4(0.0f, 0.4f, 0.4f, 1.0f);

	uboSize = sizeof(*ubo);
	return ubo;
}

void GameObject::UpdateFBOUniformBufferColor(void* uboVoid) {
	// re-implement in all child classes
}
