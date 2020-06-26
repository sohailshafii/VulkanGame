#pragma once

#include "ShipStateBehavior.h"

class ShipIdleStateBehavior : public ShipStateBehavior {
public:
	ShipIdleStateBehavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior const& motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipIdleStateBehavior";
	}

private:
	bool initialized;
	float timeWhenFireStateBegins;
};
