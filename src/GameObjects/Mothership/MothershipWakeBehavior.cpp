#include "MothershipWakeBehavior.h"
#include "MothershipIdleStateBehavior.h"
#include "MothershipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

MothershipWakeBehavior::MothershipWakeBehavior() {
	wakeTime = -1.0f;
}

ShipStateBehavior* MothershipWakeBehavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	if (wakeTime < 0.0f) {
		InitializeIfRequired(motherShip, time);
	}

	ShipStateBehavior* nextShipState = this;
	
	float endTime = wakeTime + spawnDuration;
	if (time < endTime) {

	}
	else {
		nextShipState = new MothershipIdleStateBehavior();
		motherShip.SetWorldPosition(originalMothershipPosition);
	}

	return nextShipState;
}

void MothershipWakeBehavior::InitializeIfRequired(MothershipBehavior const & motherShip,
	float time) {
	wakeTime = time;
	originalMothershipPosition = motherShip.GetWorldPosition();
}

