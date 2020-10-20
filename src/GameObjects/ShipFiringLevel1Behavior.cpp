#include "ShipFiringLevel1Behavior.h"
#include "ShipFiringLevel2Behavior.h"
#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

const float ShipFiringLevel1Behavior::timeUntilNextSpawn =1.5f;

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
		if ((rand() % 200) < 150) {
			nextShipState = new ShipIdleStateBehavior();
		}
		else {
			nextShipState = new ShipFiringLevel2Behavior();
		}
	}

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		-0.1f * deltaTime, axisOfRotation);
	motherShip.SetModelMatrix(modelMatrix);

	return nextShipState;
}

void ShipFiringLevel1Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 10) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
	}
}

void ShipFiringLevel1Behavior::SpawnPawnBasedOnTime(
	MothershipBehavior & motherShip,
	float time) {
	if (time > nextTimeToSpawnPawn) {
		motherShip.SpawnPawn();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
