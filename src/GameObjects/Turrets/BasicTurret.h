#pragma once

#include "Gameobjects/GameObject.h"

/// <summary>
/// The cheapest turret available.
/// </summary>
class BasicTurret : public GameObject {
public:
	BasicTurret(
		Scene* const scene, std::shared_ptr<GameObjectBehavior> behavior,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader,
		VkCommandPool commandPool,
		glm::mat4 const& localToWorldTransform);

private:
	std::shared_ptr<GameObject> AddSubMeshAndReturnGameObject(
		std::shared_ptr<Material> const& material,
		std::shared_ptr<Model> const& baseModel,
		std::shared_ptr<GameObjectBehavior> const& behavior,
		glm::mat4 const& relativeTransform,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader,
		VkCommandPool commandPool,
		std::string const& name,
		GameObject* parent);
};

