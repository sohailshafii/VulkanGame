#pragma once

#include "Gameobjects/GameObject.h"

// A turret containing many child game objects
public StarterTurret : public GameObject {
public:
	StarterTurret(std::shared_ptr<GameObjectBehavior> behavior,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);
private:
};

