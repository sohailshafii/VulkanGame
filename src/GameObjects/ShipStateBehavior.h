#pragma once

class ShipStateBehavior {
public:
	enum class State {
		Idle,
		FiringPawnsLevel1,
		FiringPawnsLevel2,
		FiringPawnsLevel3
	};

	virtual ~ShipStateBehavior() {
	}

	virtual State UpdateAndGetNextState(float time, float deltaTime) = 0;
private:
};

