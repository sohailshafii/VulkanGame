#pragma once 

#include "ShipStateBehavior.h"

class ShipFiringLevel1Behavior : public ShipStateBehavior {
public:
	virtual ShipStateBehavior* UpdateAndGetNextState(
		float time, float deltaTime) override;
};
