#pragma once

#include "ShipStateBehavior.h"

class ShipFiringLevel2Behavior : public ShipStateBehavior {
public:
	virtual ShipStateBehavior* UpdateAndGetNextState(
		float time, float deltaTime) override;
};
