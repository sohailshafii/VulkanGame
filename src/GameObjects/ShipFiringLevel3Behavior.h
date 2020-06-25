#pragma once

#include "ShipStateBehavior.h"

class ShipFiringLevel3Behavior : public ShipStateBehavior {
public:
	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior const& motherShip,
		float time, float deltaTime) override;
};
