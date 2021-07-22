
#include "BasicTurret.h"

BasicTurret::BasicTurret(std::shared_ptr<GameObjectBehavior> behavior,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) : GameObject(behavior, nullptr, nullptr) {
	// TODO
}