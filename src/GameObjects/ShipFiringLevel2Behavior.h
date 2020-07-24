#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class ShipFiringLevel2Behavior : public ShipStateBehavior {
public:
	ShipFiringLevel2Behavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipFiringLevel2Behavior";
	}

private:
	glm::mat4 modelMatrix;
	glm::vec3 axisOfRotation;
};
