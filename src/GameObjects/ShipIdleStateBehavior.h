#pragma once

#include "ShipStateBehavior.h"

class ShipIdleStateBehavior : public ShipStateBehavior {
public:
	virtual State UpdateAndGetNextState(float time, float deltaTime) override;
};
