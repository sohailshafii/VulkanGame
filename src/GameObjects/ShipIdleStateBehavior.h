#pragma once

#include "ShipStateBehavior.h"

class ShipIdleStateBehavior : public ShipStateBehavior {
public:
	ShipIdleStateBehavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		float time, float deltaTime) override;

private:
	bool initialized;
	float timeWhenFireStateBegins;
};
