#pragma once 

#include "ShipStateBehavior.h"

class ShipFiringLevel1Behavior : public ShipStateBehavior {
public:
	virtual State UpdateAndGetNextState(float time, float deltaTime) override;
};
