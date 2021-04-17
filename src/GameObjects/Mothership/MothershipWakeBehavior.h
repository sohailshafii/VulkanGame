#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class MothershipWakeBehavior : public ShipStateBehavior {
public:
	MothershipWakeBehavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "MothershipWakeBehavior";
	}

private:
};
