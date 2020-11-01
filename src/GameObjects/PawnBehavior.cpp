#include "PawnBehavior.h"
#include "GameObject.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float PawnBehavior::acceleration = 0.1f;
const float PawnBehavior::maxVelocityMagnitude = 2.0f;

PawnBehavior::PawnBehavior() : currentVelocity(0.0f) {
}

PawnBehavior::PawnBehavior(Scene* const scene,
						   glm::vec3 const & initialForwardVec)
	: GameObjectBehavior(scene), currentVelocity(0.0f),
	destroyed(false), initialized(false) {
		this->initialForwardVec = initialForwardVec;
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

	glm::vec3 pawnPosition = GetWorldPosition();

	if (IsCloseToPlayer(playerGameObject, pawnPosition)) {
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	ComputeHeadingDir(playerGameObject);

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

bool PawnBehavior::IsCloseToPlayer(std::shared_ptr<GameObject>
								   const & playerGameObject,
								   glm::vec3 const & pawnPosition) {
	auto playerWorldPosition =
		playerGameObject->GetWorldPosition();
	auto headingToPlayer = playerWorldPosition -
		pawnPosition;
	// TODO: use real physics engine to do intersections
	return glm::length(headingToPlayer) < 0.001f;
}

void PawnBehavior::ComputeHeadingDir(std::shared_ptr<GameObject>
									 const & playerGameObject) {
	glm::vec3 pawnPosition = GetWorldPosition();
	// do this here because in constructor our model
	// matrix is not ready yet
	if (!initialized) {
		auto playerWorldPosition =
			playerGameObject->GetWorldPosition();
		initialVectorToPlayer = playerWorldPosition -
			pawnPosition;
		initialVectorToPlayer = glm::normalize(initialVectorToPlayer);
		initialized = true;
	}
}

/*glm::vec3 PawnBehavior::ComputeCurrentPawnPosition(std::shared_ptr<GameObject>
												   const & playerGameObject) {
	
	
}*/
