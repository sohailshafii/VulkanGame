#include "MothershipIdleStateBehavior.h"
#include "MothershipFiringLevel1Behavior.h"
#include "MothershipBehavior.h"
#include "GameObjects/GameObject.h"
#include <cstdlib>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

MothershipIdleStateBehavior::MothershipIdleStateBehavior() {
	initialized = false;
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
}

ShipStateBehavior* MothershipIdleStateBehavior::UpdateAndGetNextState(
	MothershipBehavior & motherShip,
	float time, float deltaTime) {
	if (!initialized) {
		timeWhenFireStateBegins = time + rand() % 10 + 3;
		initialized = true;
	}

	ShipStateBehavior* nextShipState = this;
	if (timeWhenFireStateBegins < time) {
		nextShipState = new MothershipFiringLevel1Behavior();
	}

	modelMatrix = glm::rotate(motherShip.GetGameObject()->GetLocalTransform(),
		0.1f * deltaTime, axisOfRotation);
	motherShip.GetGameObject()->SetLocalTransform(modelMatrix);

	return nextShipState;
}
