#pragma once

#include "ShipStateBehavior.h"
#include <glm/glm.hpp>

class ShipIdleStateBehavior : public ShipStateBehavior {
public:
	ShipIdleStateBehavior();

	virtual ShipStateBehavior* UpdateAndGetNextState(
		MothershipBehavior & motherShip,
		float time, float deltaTime) override;

	virtual std::string GetDescriptiveName() const override {
		return "ShipIdleStateBehavior";
	}

private:
	bool initialized;
	float timeWhenFireStateBegins;

	glm::mat4 modelMatrix;
	glm::vec3 axisOfRotation;
};
