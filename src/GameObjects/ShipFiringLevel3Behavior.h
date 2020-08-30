#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class ShipFiringLevel3Behavior : public ShipStateBehavior {
public:
	ShipFiringLevel3Behavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipFiringLevel3Behavior";
	}

private:
	float timeToSwitchState;
	float nextTimeToSpawnPawn;
	glm::mat4 modelMatrix;
	glm::vec3 axisOfRotation;

	void InitializeIfNecessary(float time);
};
