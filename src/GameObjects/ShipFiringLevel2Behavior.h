#pragma once

#include "ShipStateBehavior.h"

class ShipFiringLevel2Behavior : public ShipStateBehavior {
public:
	virtual State UpdateAndGetNextState(float time, float deltaTime) override;
};
