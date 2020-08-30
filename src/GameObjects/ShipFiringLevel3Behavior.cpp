#include "ShipFiringLevel3Behavior.h"
#include "ShipIdleStateBehavior.h"
#include "ShipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

ShipFiringLevel3Behavior::ShipFiringLevel3Behavior() {
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* ShipFiringLevel3Behavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	InitializeIfNecessary(time);

	modelMatrix = glm::rotate(motherShip.GetModelMatrix(),
		-0.75f * deltaTime, axisOfRotation);
	motherShip.SetModelMatrix(modelMatrix);

	ShipStateBehavior* nextShipState = this;
	if (timeToSwitchState < time) {
		if ((rand() % 200) < 180) {
			nextShipState = new ShipIdleStateBehavior();
		}
		else {
			nextShipState = new ShipFiringLevel1Behavior();
		}
	}

	return nextShipState;
}

void ShipFiringLevel3Behavior::InitializeIfNecessary(float time) {
	if (timeToSwitchState < 0.0f) {
		timeToSwitchState = time + (float)(rand() % 2) + 10.0f;
		nextTimeToSpawnPawn = time + (float)(rand() % 3) + 1.5f;
	}
}
