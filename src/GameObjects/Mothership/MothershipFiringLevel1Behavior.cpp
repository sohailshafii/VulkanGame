#include "MothershipFiringLevel1Behavior.h"
#include "MothershipFiringLevel2Behavior.h"
#include "MothershipBehavior.h"
#include "MothershipIdleStateBehavior.h"
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

const float MothershipFiringLevel1Behavior::timeUntilNextSpawn =1.5f;

MothershipFiringLevel1Behavior::MothershipFiringLevel1Behavior() {
	timeToSwitchState = -1.0f;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* MothershipFiringLevel1Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);
	SpawnPawnBasedOnTime(motherShip, time);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		if ((rand() % 200) < 150) {
			nextShipState = new MothershipIdleStateBehavior();
		}
		else {
			nextShipState = new MothershipFiringLevel2Behavior();
		}
	}

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		-0.1f * deltaTime, axisOfRotation);
	motherShip.SetModelMatrix(modelMatrix);

	return nextShipState;
}

void MothershipFiringLevel1Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 10) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
	}
}

void MothershipFiringLevel1Behavior::SpawnPawnBasedOnTime(
	MothershipBehavior & motherShip,
	float time) {
	if (time > nextTimeToSpawnPawn) {
		motherShip.SpawnPawn();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
