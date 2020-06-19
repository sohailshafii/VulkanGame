#include "ShipIdleStateBehavior.h"

ShipStateBehavior::State ShipIdleStateBehavior::UpdateAndGetNextState(
	float time, float deltaTime) {
	return State::Idle;
}
