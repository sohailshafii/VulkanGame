#pragma once

class MothershipBehavior;

class ShipStateBehavior {
public:
	virtual ~ShipStateBehavior() {
	}

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior const & motherShip,
		float time, float deltaTime) = 0;
private:
};

