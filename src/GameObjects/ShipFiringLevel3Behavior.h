#pragma once

#include "ShipStateBehavior.h"

class ShipFiringLevel3Behavior : public ShipStateBehavior {
public:
	virtual State UpdateAndGetNextState(float time, float deltaTime) override;
};
