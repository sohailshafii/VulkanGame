#include "ShipIdleStateBehavior.h"
#include "ShipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#include <cstdlib>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

ShipIdleStateBehavior::ShipIdleStateBehavior() {
	initialized = false;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* ShipIdleStateBehavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	if (!initialized) {
		timeWhenFireStateBegins = time + rand() % 10 + 3;
		initialized = true;
	}

	ShipStateBehavior* nextShipState = this;
	if (timeWhenFireStateBegins < time) {
		nextShipState = new ShipFiringLevel1Behavior();
	}

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		0.1f * deltaTime, axisOfRotation);
	//motherShip.SetModelMatrix(modelMatrix);

	return nextShipState;
}
