#include "FontGameObjectBehavior.h"
#include "DescriptorSetFunctions.h"
#include "Common.h"

GameObjectBehavior::BehaviorStatus FontGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	return GameObjectBehavior::BehaviorStatus::Normal;
}

// This assumes that our material is of a certain type (font shader)
// so the UBO return is based on that
void* FontGameObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = color;

	uboSize = sizeof(*ubo);
	return ubo;
}

void* FontGameObjectBehavior::CreateUniformBufferModelViewProjColor(
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
