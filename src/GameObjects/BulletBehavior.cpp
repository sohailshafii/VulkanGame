#include "BulletBehavior.h"
#include "PawnBehavior.h"
#include "MothershipBehavior.h"
#include "GameObject.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float BulletBehavior::acceleration = 0.2f;
const float BulletBehavior::maxVelocityMagnitude = 4.0f;

BulletBehavior::BulletBehavior() : currentVelocity(0.0f) {
}

BulletBehavior::BulletBehavior(Scene* const scene,
	glm::vec3 const& velocityVector, float maxDistance)
	: GameObjectBehavior(scene), currentVelocity(0.0f),
		velocityVector(velocityVector), distanceTraveled(0.0f),
		maxDistance(maxDistance), destroyed(false) {
	this->velocityVector = glm::normalize(this->velocityVector);
}
	
BulletBehavior::~BulletBehavior() {
}

GameObjectBehavior::BehaviorStatus BulletBehavior::UpdateSelf(float time,
	float deltaTime) {
	if (destroyed) {
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	if (scene == nullptr) {
		return GameObjectBehavior::BehaviorStatus::Normal;
	}

	currentVelocity += acceleration * deltaTime;
	if (currentVelocity > maxVelocityMagnitude) {
		currentVelocity = maxVelocityMagnitude;
	}
	else if (currentVelocity < -maxVelocityMagnitude) {
		currentVelocity = -maxVelocityMagnitude;
	}
	glm::vec3 bulletPosition = GetWorldPosition();
	bulletPosition += currentVelocity * velocityVector;
	distanceTraveled += currentVelocity;
	if (distanceTraveled > maxDistance) {
		destroyed = true;
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}
	CheckForCollisions(bulletPosition);
	
	if (destroyed) {
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	modelMatrix[3][0] = bulletPosition[0];
	modelMatrix[3][1] = bulletPosition[1];
	modelMatrix[3][2] = bulletPosition[2];

	return GameObjectBehavior::BehaviorStatus::Normal;
}

void BulletBehavior::CheckForCollisions(glm::vec3 const & bulletPosition) {
	std::vector<std::shared_ptr<GameObject>>& gameObjects =
		scene->GetGameObjects();
	for (std::shared_ptr<GameObject> const& gameObject : gameObjects) {
		GameObjectBehavior* behavInst = gameObject->GetGameObjectBehavior();
		PawnBehavior* pawnBehav = dynamic_cast<PawnBehavior*>(behavInst);
		if (pawnBehav != nullptr) {
			auto pawnPos = pawnBehav->GetWorldPosition();
			auto vecToPawnPos = bulletPosition - pawnPos;
			if (glm::length(vecToPawnPos) < 2.0f) {
				destroyed = true;
				pawnBehav->Destroy();
				break;
			}
		}

		// there might be cases where we might hit mothership first in
		// loop even though it is further away than a pawn. but that's
		// unlikely. until we have real physics this is OK
		MothershipBehavior* motherBehav =
			dynamic_cast<MothershipBehavior*>(behavInst);
		if (motherBehav != nullptr) {
			if (motherBehav->TakeDamageIfHit(10, bulletPosition)) {
				destroyed = true;
				break;
			}
		}
	}
}
