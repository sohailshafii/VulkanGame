#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

MothershipBehavior::MothershipBehavior() {
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	currentShipStateBehavior = new ShipIdleStateBehavior();
	onSpawnedGameObjectSubscriber = nullptr;
}

MothershipBehavior::~MothershipBehavior() {
	if (currentShipStateBehavior != nullptr) {
		delete currentShipStateBehavior;
	}
}

void MothershipBehavior::UpdateSelf(float time, float deltaTime) {
	UpdateStateMachine(time, deltaTime);
	modelMatrix = glm::rotate(modelMatrix, 0.1f*deltaTime, axisOfRotation);
}

void MothershipBehavior::RegisterSpawnedGameObjectSubscriber(
	SpawnedGameObjectDelegate Subscriber) {
	onSpawnedGameObjectSubscriber = Subscriber;
}

void MothershipBehavior::ClearSpawnedGameObjectSubscribers() {
	onSpawnedGameObjectSubscriber = nullptr;
}

void MothershipBehavior::UpdateStateMachine(float time, float deltaTime) {
	auto nextShipState = currentShipStateBehavior->UpdateAndGetNextState(time, deltaTime);
}

