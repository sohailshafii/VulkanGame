#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include "GameObjectCreationUtilFuncs.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

MothershipBehavior::MothershipBehavior(Scene* const scene)
	: GameObjectBehavior(scene) {
	Initialize();
}

MothershipBehavior::MothershipBehavior()
	: GameObjectBehavior()
{
	Initialize();
}

MothershipBehavior::~MothershipBehavior() {
	if (currentShipStateBehavior != nullptr) {
		delete currentShipStateBehavior;
	}
}

void MothershipBehavior::UpdateSelf(float time, float deltaTime) {
	UpdateStateMachine(time, deltaTime);
	modelMatrix = glm::rotate(modelMatrix, 0.1f*deltaTime, axisOfRotation);
}

void MothershipBehavior::SpawnGameObject() const {
	scene->SpawnGameObject("Pawn");
}

void MothershipBehavior::Initialize() {
	axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	currentShipStateBehavior = new ShipIdleStateBehavior();
}

void MothershipBehavior::UpdateStateMachine(float time, float deltaTime) {
	auto nextShipState = currentShipStateBehavior->UpdateAndGetNextState(
		*this, time, deltaTime);
	if (nextShipState != currentShipStateBehavior) {
		std::cout << "Switch state from " <<
			currentShipStateBehavior->GetDescriptiveName() << " to "
			<< nextShipState->GetDescriptiveName() << ".\n";
		delete currentShipStateBehavior;
		currentShipStateBehavior = nextShipState;
	}
}

