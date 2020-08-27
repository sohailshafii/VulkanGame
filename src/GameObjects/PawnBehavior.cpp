#include "PawnBehavior.h"
#include "GameObject.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float PawnBehavior::acceleration = 0.1f;
const float PawnBehavior::maxVelocityMagnitude = 2.0f;

PawnBehavior::PawnBehavior() : currentVelocity(0.0f) {
}

PawnBehavior::PawnBehavior(Scene* const scene) 
	: GameObjectBehavior(scene), currentVelocity(0.0f) {
	int breakVar;
	breakVar = 1;
}
	
PawnBehavior::~PawnBehavior() {
}

GameObjectBehavior::BehaviorStatus PawnBehavior::UpdateSelf(float time,
	float deltaTime) {
	if (scene == nullptr) {
		return GameObjectBehavior::BehaviorStatus::Normal;
	}

	auto returnVal = GameObjectBehavior::BehaviorStatus::Normal;
	auto playerGameObject = scene->GetPlayerGameObject();
	if (playerGameObject != nullptr) {
		auto playerWorldPosition =
			playerGameObject->GetWorldPosition();
		glm::vec3 pawnPosition = GetWorldPosition();
		auto vectorToPlayer = playerWorldPosition -
			pawnPosition;
		glm::normalize(vectorToPlayer);
		// figure out which side of the plane we are on
		// assume player is on plane, normal vector to player
		float planeDistance = -glm::dot(vectorToPlayer, playerWorldPosition);
		float pawnPositionPlaneDist = glm::dot(vectorToPlayer,
			pawnPosition) + planeDistance;
		float planeDistSignBefore = pawnPositionPlaneDist < 0.0f ? -1.0f : 1.0f;

		// semi-implicit euler
		currentVelocity += acceleration * deltaTime;
		if (currentVelocity > maxVelocityMagnitude) {
			currentVelocity = maxVelocityMagnitude;
		}
		else if (currentVelocity < -maxVelocityMagnitude) {
			currentVelocity = -maxVelocityMagnitude;
		}
		pawnPosition += currentVelocity * vectorToPlayer;
		// did we overshoot? i.e. did we go on other side of plane
		float pawnPositionPlaneDistAfter = glm::dot(vectorToPlayer,
			pawnPosition) + planeDistance;
		// we went on other side of plane if signs of distances differ
		float planeDistSignAfter = pawnPositionPlaneDistAfter < 0.0f ?
			-1.0f : 1.0f;
		if (planeDistSignBefore * planeDistSignAfter < 0.0f) {
			pawnPosition = playerWorldPosition;
			returnVal = GameObjectBehavior::BehaviorStatus::Destroyed;
		}

		modelMatrix[3][0] = pawnPosition[0];
		modelMatrix[3][1] = pawnPosition[1];
		modelMatrix[3][2] = pawnPosition[2];
	}

	return returnVal;
}
