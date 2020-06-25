#include "ShipFiringLevel2Behavior.h"
#include "MothershipBehavior.h"

ShipStateBehavior* ShipFiringLevel2Behavior::UpdateAndGetNextState(
	MothershipBehavior const& motherShip,
	float time, float deltaTime) {
	return this;
}
