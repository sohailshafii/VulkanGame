#include "ShipFiringLevel2Behavior.h"

ShipStateBehavior::State ShipFiringLevel2Behavior::UpdateAndGetNextState(
	float time, float deltaTime) {
	return State::FiringPawnsLevel2;
}
