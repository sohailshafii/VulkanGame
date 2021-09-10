#pragma once

#include "Gameobjects/MeshGameObject.h"

class MothershipBehavior;

class Mothership : public MeshGameObject {
public:
	Mothership(std::shared_ptr<GameObjectBehavior> behavior,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool,
		std::shared_ptr<Model> const& model,
		std::shared_ptr<Material> const& material,
		glm::mat4 const & localToWorldTransform);

protected:
	virtual void* CreateUniformBufferModelViewProjRipple(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime) override;

private:
	std::shared_ptr<MothershipBehavior> mothershipBehavior;
};
