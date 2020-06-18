#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

MothershipBehavior::MothershipBehavior()
{
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	CurrentShipState = ShipState::Idle;
}

MothershipBehavior::~MothershipBehavior()
{
}

void MothershipBehavior::UpdateSelf(float time, float deltaTime) {
	UpdateStateMachine();
	modelMatrix = glm::rotate(modelMatrix, 0.1f*deltaTime, axisOfRotation);
}

void MothershipBehavior::RegisterSpawnedGameObjectSubscriber(
	SpawnedGameObjectDelegate Subscriber) {
	OnSpawnedGameObjectSubscriber = Subscriber;
}

void MothershipBehavior::ClearSpawnedGameObjectSubscribers() {
	OnSpawnedGameObjectSubscriber = nullptr;
}

void MothershipBehavior::UpdateStateMachine() {
	switch (CurrentShipState) {
		case ShipState::Idle:
			break;
		case ShipState::FiringPawnsLevel1:
			break;
		case ShipState::FiringPawnsLevel2:
			break;
		case ShipState::FiringPawnsLevel3:
			break;
	}
}
