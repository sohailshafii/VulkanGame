#include "GameObjectBehavior.h"
#include "GameObject.h"
#include "DescriptorSetFunctions.h"
#include "Common.h"

void* GameObjectBehavior::CreateVertUBOData(size_t& uboSize,
	VkExtent2D const& swapChainExtent, const glm::mat4& viewMatrix,
	float time, float deltaTime) {
	switch (gameObject->GetMaterialType())
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

void GameObjectBehavior::UpdateVertUBOData(void* vboData,
	VkExtent2D const& swapChainExtent, const glm::mat4& viewMatrix,
	float time, float deltaTime) {
	switch (gameObject->GetMaterialType())
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

void* GameObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	uboSize = sizeof(*ubo);

	switch (gameObject->GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::Text:
			return CreateFBOUniformBufferColor(uboSize);
		default:
			return nullptr;
	}
	return ubo;
}

void GameObjectBehavior::UpdateFragUBOData(void* vboData) {
	switch (gameObject->GetMaterialType())
	{
	case DescriptorSetFunctions::MaterialType::UnlitColor:
	case DescriptorSetFunctions::MaterialType::Text:
		return UpdateFBOUniformBufferColor(vboData);
	}
}

void* GameObjectBehavior::CreateUniformBufferModelViewProj(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProj* ubo =
		new UniformBufferObjectModelViewProj();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);

	uboSize = sizeof(UniformBufferObjectModelViewProj);// *ubo);
	return ubo;
}

void* GameObjectBehavior::CreateUniformBufferModelViewProjRipple(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		new UniformBufferObjectModelViewProjRipple();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);

	uboSize = sizeof(*ubo);
	return ubo;
}

void* GameObjectBehavior::CreateUniformBufferModelViewProjTime(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjTime* ubo =
		new UniformBufferObjectModelViewProjTime();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
	ubo->time = time;

	uboSize = sizeof(*ubo);
	return ubo;
}

void GameObjectBehavior::UpdateUniformBufferModelViewProj(
	void* uboVoid, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProj* ubo =
		(UniformBufferObjectModelViewProj*)uboVoid;
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
}

void GameObjectBehavior::UpdateUniformBufferModelViewProjRipple(
	void* uboVoid, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		(UniformBufferObjectModelViewProjRipple*)uboVoid;
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
}

void GameObjectBehavior::UpdateUniformBufferModelViewProjTime(
	void* uboVoid, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjTime* ubo =
		(UniformBufferObjectModelViewProjTime*)uboVoid;
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
	ubo->time = time;
}

void* GameObjectBehavior::CreateFBOUniformBufferColor(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = glm::vec4(0.0f, 0.4f, 0.4f, 1.0f);

	uboSize = sizeof(*ubo);
	return ubo;
}

void GameObjectBehavior::UpdateFBOUniformBufferColor(void* uboVoid) {
	// re-implement in all child classes
}
