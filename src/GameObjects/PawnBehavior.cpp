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

void PawnBehavior::UpdateSelf(float time, float deltaTime) {
	if (scene != nullptr) {
		auto playerGameObject = scene->GetPlayerGameObject();
		if (playerGameObject != nullptr) {
			auto playerWorldPosition =
				playerGameObject->GetWorldPosition();
			glm::vec3 pawnPosition(modelMatrix[0][3], modelMatrix[1][3],
				modelMatrix[2][3]);
			auto vectorToPlayer = playerWorldPosition -
				pawnPosition;
			glm::normalize(vectorToPlayer);

			// semi-implicit euler
			currentVelocity += acceleration * deltaTime;
			if (currentVelocity > maxVelocityMagnitude) {
				currentVelocity = maxVelocityMagnitude;
			}
			else if (currentVelocity < -maxVelocityMagnitude) {
				currentVelocity = -maxVelocityMagnitude;
			}
			pawnPosition += currentVelocity * vectorToPlayer;
			modelMatrix[0][3] = pawnPosition[0];
			modelMatrix[1][3] = pawnPosition[1];
			modelMatrix[2][3] = pawnPosition[2];
		}
	}
}
