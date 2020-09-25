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
	: GameObjectBehavior(scene), currentVelocity(0.0f),
	destroyed(false), initialized(false) {
}
	
PawnBehavior::~PawnBehavior() {
}

GameObjectBehavior::BehaviorStatus PawnBehavior::UpdateSelf(float time,
	float deltaTime) {
	if (destroyed) {
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	if (scene == nullptr) {
		return GameObjectBehavior::BehaviorStatus::Normal;
	}

	auto playerGameObject = scene->GetPlayerGameObject();
	if (playerGameObject == nullptr) {
		return GameObjectBehavior::BehaviorStatus::Normal;
	}

	// do this here because in constructor our model
	// matrix is not ready yet
	if (!initialized) {
		auto playerWorldPosition =
			playerGameObject->GetWorldPosition();
		glm::vec3 pawnPosition = GetWorldPosition();
		initialVectorToPlayer = playerWorldPosition -
			pawnPosition;
		initialVectorToPlayer = glm::normalize(initialVectorToPlayer);
		initialized = true;
	}

	auto playerWorldPosition =
		playerGameObject->GetWorldPosition();
	glm::vec3 pawnPosition = GetWorldPosition();
	auto headingToPlayer = playerWorldPosition -
		pawnPosition;

	// TODO: use real physics to do intersections
	if (glm::length(headingToPlayer) < 0.001f) {
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	// semi-implicit euler
	currentVelocity += acceleration * deltaTime;
	if (currentVelocity > maxVelocityMagnitude) {
		currentVelocity = maxVelocityMagnitude;
	}
	else if (currentVelocity < -maxVelocityMagnitude) {
		currentVelocity = -maxVelocityMagnitude;
	}
	pawnPosition += currentVelocity * initialVectorToPlayer;

	modelMatrix[3][0] = pawnPosition[0];
	modelMatrix[3][1] = pawnPosition[1];
	modelMatrix[3][2] = pawnPosition[2];

	return GameObjectBehavior::BehaviorStatus::Normal;
}
