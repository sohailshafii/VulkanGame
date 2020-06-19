#include "ShipFiringLevel3Behavior.h"

ShipStateBehavior::State ShipFiringLevel3Behavior::UpdateAndGetNextState(
	float time, float deltaTime) {
	return State::FiringPawnsLevel3;
}
