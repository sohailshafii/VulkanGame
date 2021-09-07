#include "MothershipFiringLevel3Behavior.h"
#include "MothershipIdleStateBehavior.h"
#include "MothershipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#include "GameObjects/GameObject.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const float MothershipFiringLevel3Behavior::timeUntilNextSpawn = 0.7f;

MothershipFiringLevel3Behavior::MothershipFiringLevel3Behavior() {
	timeToSwitchState = -1.0f;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* MothershipFiringLevel3Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);
	SpawnPawnBasedOnTime(motherShip, time);

	modelMatrix = glm::rotate(motherShip.GetGameObject()->GetLocalTransform(),
		-0.75f * deltaTime, axisOfRotation);
	motherShip.GetGameObject()->SetLocalTransform(modelMatrix);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		if ((rand() % 200) < 180) {
			nextShipState = new MothershipIdleStateBehavior();
		}
		else {
			nextShipState = new MothershipFiringLevel1Behavior();
		}
	}

	return nextShipState;
}

void MothershipFiringLevel3Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 2) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
	}
}

void MothershipFiringLevel3Behavior::SpawnPawnBasedOnTime(
	MothershipBehavior& motherShip,
	float time) {
	if (time > nextTimeToSpawnPawn) {
		motherShip.SpawnPawn();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + timeUntilNextSpawn;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
