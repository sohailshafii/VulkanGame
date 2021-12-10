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

	void SetGunTransformForSphericalCoords(float azim, float polar, bool sLerp = false);
	void SetGunLookRotation(glm::vec3 const& lookAtPoint, bool sLerp = false);

	glm::vec3 GetCurrentLookAtPoint() const {
		return currentLookAtPoint;
	}

	virtual void UpdateState(float time, float deltaTime) override;

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

	glm::mat4 GetRotationForLookAtPoint(glm::vec3 const& lookAtPoint);
	glm::mat4 GetGunPositionalTransform(glm::vec3 const& forwardVector);
	glm::mat4 GetTransformLookAtPoint(glm::vec3 const & lookAtPoint);
	glm::mat4 GetTransformForSphericalCoords(float azim, float polar,
		glm::vec3 & lookAtPoint);

	std::shared_ptr<GameObject> turretBase;
	std::shared_ptr<GameObject> turretBody;
	std::shared_ptr<GameObject> turretTop;
	std::shared_ptr<GameObject> turretGun;
	glm::vec3 gunCenter;
	glm::vec3 currentLookAtPoint;

	// in case slerp is request for rotational changes
	CommonMath::Quaternion startRotation;
	CommonMath::Quaternion targetRotation;
	bool isDoingSLerp;
	float currentT;

	static const float turretWidth;
	static const float turretDepth;
	static const float turretHeight;
	static const float topRadius;
	static const float sLerpSpeed;
};

