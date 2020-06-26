#pragma once 

#include "ShipStateBehavior.h"

class ShipFiringLevel1Behavior : public ShipStateBehavior {
public:
	ShipFiringLevel1Behavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior const& motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipFiringLevel1Behavior";
	}

private:
	float timeToSwitchState;
	float nextTimeToSpawnPawn;

	void InitializeIfNecessary(float time);
	void SpawnPawnBasedOnTime(MothershipBehavior
		const& motherShip, float time);
};
