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

	void SetGunTransformForSphericalCoords(float azim, float polar);

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

	glm::mat4 GetGunTransformForSphericalCoords(float azim, float polar);

	std::shared_ptr<GameObject> turretBase;
	std::shared_ptr<GameObject> turretBody;
	std::shared_ptr<GameObject> turretTop;
	std::shared_ptr<GameObject> turretGun;
	glm::vec3 gunCenter;

	static const float turretWidth;
	static const float turretDepth;
	static const float turretHeight;
	static const float topRadius;
};

