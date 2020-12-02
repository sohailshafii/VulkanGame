#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class MothershipIdleStateBehavior : public ShipStateBehavior {
public:
	MothershipIdleStateBehavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "MothershipIdleStateBehavior";
	}

private:
	bool initialized;
	float timeWhenFireStateBegins;

	glm::mat4 modelMatrix;
	glm::vec3 axisOfRotation;
};
