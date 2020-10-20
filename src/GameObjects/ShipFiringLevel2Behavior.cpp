#include "ShipFiringLevel2Behavior.h"
#include "ShipIdleStateBehavior.h"
#include "ShipFiringLevel3Behavior.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const float ShipFiringLevel2Behavior::timeUntilNextSpawn = 1.1f;

ShipFiringLevel2Behavior::ShipFiringLevel2Behavior() {
	timeToSwitchState = -1.0f;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 1.0f));
}

ShipStateBehavior* ShipFiringLevel2Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);
	SpawnPawnBasedOnTime(motherShip, time);

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		-0.5f * deltaTime, axisOfRotation);
	motherShip.SetModelMatrix(modelMatrix);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		if ((rand() % 200) < 150) {
			nextShipState = new ShipIdleStateBehavior();
		}
		else {
			nextShipState = new ShipFiringLevel3Behavior();
		}
	}


	return nextShipState;
}

void ShipFiringLevel2Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 3) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
	}
}

void ShipFiringLevel2Behavior::SpawnPawnBasedOnTime(
	MothershipBehavior& motherShip,
	float time) {
	if (time > nextTimeToSpawnPawn) {
		motherShip.SpawnPawn();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
