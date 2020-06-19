#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

MothershipBehavior::MothershipBehavior() {
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	currentShipState = ShipState::Idle;
	onSpawnedGameObjectSubscriber = nullptr;
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
	auto nextShipState = currentShipState;
	switch (currentShipState) {
		case ShipState::Idle:
			nextShipState = UpdateIdleAndGetNextState(time, deltaTime);
			break;
		case ShipState::FiringPawnsLevel1:
			nextShipState = UpdateLevel1AndGetNextState(time, deltaTime);
			break;
		case ShipState::FiringPawnsLevel2:
			nextShipState = UpdateLevel2AndGetNextState(time, deltaTime);
			break;
		case ShipState::FiringPawnsLevel3:
			nextShipState = UpdateLevel2AndGetNextState(time, deltaTime);
			break;
	}

	currentShipState = nextShipState;
}

MothershipBehavior::ShipState MothershipBehavior::UpdateIdleAndGetNextState(
	float time, float deltaTime) {

	return ShipState::Idle;
}

MothershipBehavior::ShipState MothershipBehavior::UpdateLevel1AndGetNextState(
	float time, float deltaTime) {

	return ShipState::FiringPawnsLevel1;
}

MothershipBehavior::ShipState MothershipBehavior::UpdateLevel2AndGetNextState(
	float time, float deltaTime) {

	return ShipState::FiringPawnsLevel2;
}

MothershipBehavior::ShipState MothershipBehavior::UpdateLevel3AndGetNextState(
	float time, float deltaTime) {

	return ShipState::FiringPawnsLevel3;
}
