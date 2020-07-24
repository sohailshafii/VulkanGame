#pragma once 

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class ShipFiringLevel1Behavior : public ShipStateBehavior {
public:
	ShipFiringLevel1Behavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipFiringLevel1Behavior";
	}

private:
	float timeToSwitchState;
	float nextTimeToSpawnPawn;
	glm::mat4 modelMatrix;
	glm::vec3 axisOfRotation;

	void InitializeIfNecessary(float time);
	void SpawnPawnBasedOnTime(MothershipBehavior
		const& motherShip, float time);
};
