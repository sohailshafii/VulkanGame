#include "ShipFiringLevel3Behavior.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

ShipFiringLevel3Behavior::ShipFiringLevel3Behavior() {
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* ShipFiringLevel3Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		-0.25f * deltaTime, axisOfRotation);
	motherShip.SetModelMatrix(modelMatrix);

	return this;
}
