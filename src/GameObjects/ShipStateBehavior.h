#pragma once

#include <string>

class MothershipBehavior;

class ShipStateBehavior {
public:
	virtual ~ShipStateBehavior() {
	}

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) = 0;

	virtual std::string GetDescriptiveName() const = 0;

private:
};

