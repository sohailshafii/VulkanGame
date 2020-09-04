#include "BulletBehavior.h"
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
	glm::vec3 const& velocityVector)
	: GameObjectBehavior(scene), currentVelocity(0.0f),
		velocityVector(velocityVector) {
	glm::normalize(velocityVector);
}
	
BulletBehavior::~BulletBehavior() {
}

GameObjectBehavior::BehaviorStatus BulletBehavior::UpdateSelf(float time,
	float deltaTime) {
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

	modelMatrix[3][0] = bulletPosition[0];
	modelMatrix[3][1] = bulletPosition[1];
	modelMatrix[3][2] = bulletPosition[2];

	return GameObjectBehavior::BehaviorStatus::Normal;
}
