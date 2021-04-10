#include "MenuSelectorObjectBehavior.h"
#include "DescriptorSetFunctions.h"
#include "Common.h"

GameObjectBehavior::BehaviorStatus MenuSelectorObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	return GameObjectBehavior::BehaviorStatus::Normal;
}

void* MenuSelectorObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = color;

	uboSize = sizeof(*ubo);
	return ubo;
}

void* MenuSelectorObjectBehavior::CreateUniformBufferModelViewProjColor(
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
	ubo->color = color;

	uboSize = sizeof(*ubo);
	return ubo;
}
