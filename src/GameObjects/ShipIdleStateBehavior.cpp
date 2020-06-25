#include "ShipIdleStateBehavior.h"
#include "ShipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#include <cstdlib>

ShipIdleStateBehavior::ShipIdleStateBehavior() {
	initialized = false;
}

ShipStateBehavior* ShipIdleStateBehavior::UpdateAndGetNextState(
	MothershipBehavior const& motherShip,
	float time, float deltaTime) {
	if (!initialized) {
		timeWhenFireStateBegins = time + rand() % 10;
		initialized = true;
	}

	ShipStateBehavior* nextShipState = this;
	if (timeWhenFireStateBegins < time) {
		nextShipState = new ShipFiringLevel1Behavior();
	}
	return nextShipState;
}
