#include "ShipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"

ShipStateBehavior* ShipFiringLevel1Behavior::UpdateAndGetNextState(
	MothershipBehavior const& motherShip,
	float time, float deltaTime) {
	return this;
}
