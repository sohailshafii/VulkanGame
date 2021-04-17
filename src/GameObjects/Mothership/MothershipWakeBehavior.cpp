#include "MothershipWakeBehavior.h"
#include "MothershipIdleStateBehavior.h"
#include "MothershipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

MothershipWakeBehavior::MothershipWakeBehavior() {
	// TODO
}

ShipStateBehavior* MothershipWakeBehavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {

	ShipStateBehavior* nextShipState = this;
	// TODO
	return nextShipState;
}

