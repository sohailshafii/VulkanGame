#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include "GameObjectCreationUtilFuncs.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

const int MothershipBehavior::maxHealth = 300;

MothershipBehavior::MothershipBehavior(Scene* const scene, float radius)
	: GameObjectBehavior(scene), radius(radius), currentHealth(maxHealth) {
	Initialize();
}

MothershipBehavior::MothershipBehavior()
	: GameObjectBehavior(), currentHealth(maxHealth)
{
	Initialize();
}

MothershipBehavior::~MothershipBehavior() {
	if (currentShipStateBehavior != nullptr) {
		delete currentShipStateBehavior;
	}
}

GameObjectBehavior::BehaviorStatus MothershipBehavior::UpdateSelf(
	float time, float deltaTime) {
	if (ripplePositions.size() > 0) {
		glm::vec3 newRipplePosition = ripplePositions.top();
		ripplePositions.pop();
		// TODO: affect material somehow; link behavior with game object
	}

	return UpdateStateMachine(time, deltaTime);
}

void MothershipBehavior::SpawnGameObject() const {
	if (scene != nullptr) {
		float randPhi = 3.14f * 0.5f * ((float)rand()/RAND_MAX);
		float randTheta = 3.14f * 2.0f * ((float)rand() / RAND_MAX);
		float adjustedRadius = radius * 0.7f;
		glm::vec3 randomPos(adjustedRadius * sin(randTheta) * sin(randPhi),
			adjustedRadius * cos(randPhi),
			adjustedRadius * cos(randTheta) * sin(randPhi));
		scene->SpawnGameObject(Scene::SpawnType::Pawn, randomPos,
			glm::vec3(0.0f, 0.0f, 1.0f));
	}
}

void MothershipBehavior::Initialize() {
	currentShipStateBehavior = new ShipIdleStateBehavior();
}

void MothershipBehavior::TakeDamage(int damage, glm::vec3 const& hitPosition) {
	glm::vec3 worldPosition = GetWorldPosition();
	glm::vec3 vectorFromCenter = glm::normalize(hitPosition - worldPosition);
	glm::vec3 surfacePoint = worldPosition + vectorFromCenter * radius;

	if (dynamic_cast<ShipIdleStateBehavior*>(currentShipStateBehavior)
		!= nullptr) {
		return;
		// TODO: react to not being able to take damage

	}
	currentHealth -= damage;
	// ripple near where damage was dealt
	ripplePositions.push(surfacePoint);
	if (currentHealth < 0) {
		currentHealth = 0;
	}
	std::cout << "Current health after taking damage: " << currentHealth << std::endl;
}

void MothershipBehavior::GetUBOInformation(void** uboData, size_t& uboSize) {
	// TODO
}

GameObjectBehavior::BehaviorStatus MothershipBehavior::UpdateStateMachine(
	float time, float deltaTime) {
	if (currentHealth == 0) {
		return GameObjectBehavior::BehaviorStatus::Destroyed;
	}

	auto nextShipState = currentShipStateBehavior->UpdateAndGetNextState(
		*this, time, deltaTime);
	if (nextShipState != currentShipStateBehavior) {
		std::cout << "Switch state from " <<
			currentShipStateBehavior->GetDescriptiveName() << " to "
			<< nextShipState->GetDescriptiveName() << ".\n";
		delete currentShipStateBehavior;
		currentShipStateBehavior = nextShipState;
	}

	return GameObjectBehavior::BehaviorStatus::Normal;
}

