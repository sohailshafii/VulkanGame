#include "MothershipFiringLevel2Behavior.h"
#include "MothershipIdleStateBehavior.h"
#include "MothershipFiringLevel3Behavior.h"
#include "MothershipBehavior.h"
#include "GameObjects/GameObject.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const float MothershipFiringLevel2Behavior::timeUntilNextSpawn = 1.1f;

MothershipFiringLevel2Behavior::MothershipFiringLevel2Behavior() {
	timeToSwitchState = -1.0f;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 1.0f));
}

ShipStateBehavior* MothershipFiringLevel2Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);
	SpawnPawnBasedOnTime(motherShip, time);

	modelMatrix = glm::rotate(motherShip.GetGameObject()->GetLocalTransform(),
		-0.5f * deltaTime, axisOfRotation);
	motherShip.GetGameObject()->SetLocalTransform(modelMatrix);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		if ((rand() % 200) < 150) {
			nextShipState = new MothershipIdleStateBehavior();
		}
		else {
			nextShipState = new MothershipFiringLevel3Behavior();
		}
	}


	return nextShipState;
}

void MothershipFiringLevel2Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 3) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
	}
}

void MothershipFiringLevel2Behavior::SpawnPawnBasedOnTime(
	MothershipBehavior& motherShip,
	float time) {
	if (time > nextTimeToSpawnPawn) {
		motherShip.SpawnPawn();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
