#include "FontGameObjectBehavior.h"
#include "DescriptorSetFunctions.h"

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
	ubo->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo->proj[1][1] *= -1; // flip Y -- opposite of opengl
	ubo->color = color;

	uboSize = sizeof(*ubo);
	return ubo;
}
