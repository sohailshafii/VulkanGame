#include "ShipFiringLevel3Behavior.h"
#include "MothershipBehavior.h"

ShipStateBehavior* ShipFiringLevel3Behavior::UpdateAndGetNextState(
	MothershipBehavior const& motherShip,
	float time, float deltaTime) {
	return this;
}
