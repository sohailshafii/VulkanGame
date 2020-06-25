#pragma once

#include "ShipStateBehavior.h"

class ShipFiringLevel3Behavior : public ShipStateBehavior {
public:
	virtual ShipStateBehavior* UpdateAndGetNextState(
		float time, float deltaTime) override;
};
