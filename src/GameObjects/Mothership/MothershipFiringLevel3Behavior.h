#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class MothershipFiringLevel3Behavior : public ShipStateBehavior {
public:
	MothershipFiringLevel3Behavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "MothershipFiringLevel3Behavior";
	}

private:
	float timeToSwitchState;
	float nextTimeToSpawnPawn;
	glm::mat4 modelMatrix;
	glm::vec3 axisOfRotation;

	static const float timeUntilNextSpawn;

	void InitializeIfNecessary(float time);
	void SpawnPawnBasedOnTime(MothershipBehavior
		& motherShip, float time);
};
