#pragma once

#include "Gameobjects/GameObject.h"
#include "Math/CommonMath.h"

class BasicTurretBehavior;

/// <summary>
/// The cheapest turret available.
/// </summary>
class BasicTurret : public GameObject {
public:
	BasicTurret(
		Scene* const scene,
		std::shared_ptr<BasicTurretBehavior> const & behavior,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader,
		VkCommandPool commandPool,
		glm::mat4 const& localToWorldTransform);

	void SetGunTransformForSphericalCoords(float azim, float polar);
	void SetGunLookRotation(glm::vec3 const& lookAtPoint);

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

	float GetGunLength() const {
		return turretDepth * 0.3f;
	}

	glm::mat4 GetTransformLookAtPoint(glm::vec3 const& lookAtPoint);
	glm::mat4 GetTransformForSphericalCoords(float azim, float polar,
		glm::vec3 & lookAtPoint);

	std::shared_ptr<GameObject> turretBase;
	std::shared_ptr<GameObject> turretBody;
	std::shared_ptr<GameObject> turretTop;
	std::shared_ptr<GameObject> turretGun;
	glm::vec3 gunCenter;
	CommonMath::Quaternion currentRotation;
	glm::vec3 currentLookPoint;

	static const float turretWidth;
	static const float turretDepth;
	static const float turretHeight;
	static const float topRadius;
};

