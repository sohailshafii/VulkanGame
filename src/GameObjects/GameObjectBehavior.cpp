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
			return CreateUniformBufferModelViewProj(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
		case DescriptorSetFunctions::MaterialType::Text:
			return CreateUniformBufferModelViewProjColor(uboSize,
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

void* GameObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	return nullptr;
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

	uboSize = sizeof(*ubo);
	return ubo;
}

void* GameObjectBehavior::CreateUniformBufferModelViewProjColor(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjColor* ubo =
		new UniformBufferObjectModelViewProjColor();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
	ubo->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	uboSize = sizeof(*ubo);
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
