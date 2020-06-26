#pragma once

#include "ShipStateBehavior.h"

class ShipFiringLevel3Behavior : public ShipStateBehavior {
public:
	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior const& motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipFiringLevel3Behavior";
	}
};
