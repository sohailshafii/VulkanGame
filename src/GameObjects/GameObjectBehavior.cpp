#include "GameObjectBehavior.h"
#include "GameObject.h"
#include "DescriptorSetFunctions.h"

void* GameObjectBehavior::GetUBOData(size_t& uboSize,
	VkExtent2D const& swapChainExtent, const glm::mat4& viewMatrix,
	float time, float deltaTime) {
	
	switch (gameObject->GetMaterialType())
	{
		case DescriptorSetFunctions::MaterialType::UnlitColor:
		case DescriptorSetFunctions::MaterialType::UnlitTintedTextured:
			return GetUniformBufferModelViewProj(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
		case DescriptorSetFunctions::MaterialType::MotherShip:
			return GetUniformBufferModelViewProjRipple(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
		default:
			return GetUniformBufferModelViewProjTime(uboSize,
				swapChainExtent, viewMatrix, time, deltaTime);
	}
}

void* GameObjectBehavior::GetUniformBufferModelViewProj(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProj* ubo =
		new UniformBufferObjectModelViewProj();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo->proj[1][1] *= -1; // flip Y -- opposite of opengl

	uboSize = sizeof(*ubo);
	return ubo;
}

void* GameObjectBehavior::GetUniformBufferModelViewProjRipple(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		new UniformBufferObjectModelViewProjRipple();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = glm::perspective(glm::radians(45.0f),
		swapChainExtent.width / (float)swapChainExtent.height,
		0.1f, 1000.0f);
	ubo->proj[1][1] *= -1; // flip Y -- opposite of opengl

	uboSize = sizeof(*ubo);
	return ubo;
}

void* GameObjectBehavior::GetUniformBufferModelViewProjTime(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjTime* ubo =
		new UniformBufferObjectModelViewProjTime();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo->proj[1][1] *= -1; // flip Y -- opposite of opengl
	ubo->time = time;

	uboSize = sizeof(*ubo);
	return ubo;
}
