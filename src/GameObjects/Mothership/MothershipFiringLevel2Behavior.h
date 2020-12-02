#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class MothershipFiringLevel2Behavior : public ShipStateBehavior {
public:
	MothershipFiringLevel2Behavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "MothershipFiringLevel2Behavior";
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
