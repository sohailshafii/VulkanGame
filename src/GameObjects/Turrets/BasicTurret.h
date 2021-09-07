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
		VkCommandPool commandPool);
private:
};

