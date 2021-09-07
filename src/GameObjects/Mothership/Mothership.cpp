
#include "Mothership.h"
#include "GameObjects/Mothership/MothershipBehavior.h"

Mothership::Mothership(std::shared_ptr<GameObjectBehavior> behavior,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool,
	std::shared_ptr<Model> const& model,
	std::shared_ptr<Material> const& material) :
		MeshGameObject(behavior, gfxDeviceManager, logicalDeviceManager,
			commandPool, model, material) {
	mothershipBehavior =
		std::dynamic_pointer_cast<MothershipBehavior>(behavior);
}

void* Mothership::CreateUniformBufferModelViewProjRipple(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		new UniformBufferObjectModelViewProjRipple();
	ubo->model = GetLocalToWorld();
	ubo->view = viewMatrix;
	ubo->proj = Common::ConstructProjectionMatrix(swapChainExtent.width,
		swapChainExtent.height);
	ubo->time = time;

	mothershipBehavior->UpdateUBOBehaviorData(ubo);

	uboSize = sizeof(*ubo);
	return ubo;
}