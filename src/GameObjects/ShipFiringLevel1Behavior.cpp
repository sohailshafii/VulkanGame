#include "ShipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

ShipFiringLevel1Behavior::ShipFiringLevel1Behavior() {
	timeToSwitchState = -1.0f;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* ShipFiringLevel1Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);
	SpawnPawnBasedOnTime(motherShip, time);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		nextShipState = new ShipIdleStateBehavior();
	}

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		-0.1f * deltaTime, axisOfRotation);
	motherShip.SetModelMatrix(modelMatrix);

	return nextShipState;
}

void ShipFiringLevel1Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 10) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + 1.5f;
	}
}

void ShipFiringLevel1Behavior::SpawnPawnBasedOnTime(
	MothershipBehavior const& motherShip,
	float time) {
	if (time > nextTimeToSpawnPawn) {
		motherShip.SpawnGameObject();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + 1.5f;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
