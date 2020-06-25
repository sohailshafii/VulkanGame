#pragma once

class ShipStateBehavior {
public:
	virtual ~ShipStateBehavior() {
	}

	virtual ShipStateBehavior* UpdateAndGetNextState(
		float time, float deltaTime) = 0;
private:
};

