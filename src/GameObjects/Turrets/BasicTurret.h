#pragma once

#include "Gameobjects/GameObject.h"

/// <summary>
/// The cheapest stationary available.
/// </summary>
class BasicTurret : public GameObject {
public:
	BasicTurret(std::shared_ptr<GameObjectBehavior> behavior,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);
private:
};

