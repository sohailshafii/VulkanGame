#include "ShipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include <iostream>

ShipFiringLevel1Behavior::ShipFiringLevel1Behavior() {
	timeToSwitchState = -1.0f;
}

ShipStateBehavior* ShipFiringLevel1Behavior::UpdateAndGetNextState(
	MothershipBehavior const& motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);
	SpawnPawnBasedOnTime(motherShip, time);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		nextShipState = new ShipIdleStateBehavior();
	}
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
		motherShip.SpawnPawnObject();
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + 1.5f;
		std::cout << "Spawned pawn at time: " << time << ".\n";
	}
}
