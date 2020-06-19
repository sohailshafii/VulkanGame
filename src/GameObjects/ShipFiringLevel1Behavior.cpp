#include "ShipFiringLevel1Behavior.h"

ShipStateBehavior::State ShipFiringLevel1Behavior::UpdateAndGetNextState(
	float time, float deltaTime) {
	return State::FiringPawnsLevel1;
}
