#include "PawnBehavior.h"
#include "GameObject.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float PawnBehavior::acceleration = 0.1f;
const float PawnBehavior::maxVelocityMagnitude = 2.0f;

PawnBehavior::PawnBehavior() : currentVelocity(0.0f),
	currentPawnState(JustCreated) {
}

PawnBehavior::PawnBehavior(Scene* const scene,
						   glm::vec3 const & initialForwardVec)
	: GameObjectBehavior(scene), currentVelocity(0.0f),
	currentPawnState(JustCreated) {
		this->currentForwardVec = initialForwardVec;
}
	
PawnBehavior::~PawnBehavior() {
}

GameObjectBehavior::BehaviorStatus PawnBehavior::UpdateSelf(float time,
	float deltaTime) {
	if (currentPawnState == Destroyed) {
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
		currentPawnState = Destroyed;
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	pawnPosition = ComputeMovement(playerGameObject, time,
		deltaTime);

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

glm::vec3 PawnBehavior::ComputeMovement(std::shared_ptr<GameObject>
										const & playerGameObject,
										float currentTime,
										float deltaTime) {
	glm::vec3 pawnPosition = GetWorldPosition();

	switch (currentPawnState) {
		case JustCreated:
			timeBeginState = currentTime;
			timeEndState = currentTime + MothershipBehavior::stalkRiseDuration;
			startPosition = pawnPosition;
			currentPawnState = Spawning;
			break;
		case Spawning:
			if (currentTime > timeEndState) {
				auto playerWorldPosition =
					playerGameObject->GetWorldPosition();
				currentForwardVec = glm::normalize(playerWorldPosition -
					pawnPosition);
				currentPawnState = HeadingToPlayer;
			}
			else {
				float lerpVal = (currentTime - timeBeginState) /
					(timeEndState - timeBeginState);
				if (lerpVal > 1.0f) {
					lerpVal = 1.0f;
				}
				
				glm::vec3 endPos = startPosition +
					// based on equation found in vertex shader
					// for mothership...for max displacement
					// we use a constant that is slightly larger than
					// what the shader uses (1.0/0.2 or 8.0)
					currentForwardVec * 10.0f;
				pawnPosition = startPosition * (1.0f - lerpVal)
					+ endPos * lerpVal;
			}
			break;
		case HeadingToPlayer:
			// semi-implicit euler
			currentVelocity += acceleration * deltaTime;
			if (currentVelocity > maxVelocityMagnitude) {
				currentVelocity = maxVelocityMagnitude;
			}
			else if (currentVelocity < -maxVelocityMagnitude) {
				currentVelocity = -maxVelocityMagnitude;
			}
			pawnPosition += currentVelocity * currentForwardVec;
			break;
	}

	return pawnPosition;
}

/*glm::vec3 PawnBehavior::ComputeCurrentPawnPosition(std::shared_ptr<GameObject>
												   const & playerGameObject) {
	
	
}*/
