#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include "GameObjectCreationUtilFuncs.h"
#include "DescriptorSetFunctions.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

const int MothershipBehavior::maxHealth = 300;
const float MothershipBehavior::maxRippleDurationSeconds = 3.0f;

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
	if (ripples.size() > 0) {
		//glm::vec3 newRipplePosition = ripplePositions.top();
		//ripplePositions.pop();
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
	if (currentHealth == 0) {
		return;
	}
	
	glm::vec3 worldPosition = GetWorldPosition();
	glm::vec3 vectorFromCenter = glm::normalize(hitPosition - worldPosition);
	glm::vec3 surfacePoint = worldPosition + vectorFromCenter * radius;
	glm::mat4 worldToModelMat = glm::inverse(modelMatrix);
	glm::vec4 surfacePointLocal = worldToModelMat * glm::vec4(surfacePoint, 1.0f);

	if (dynamic_cast<ShipIdleStateBehavior*>(currentShipStateBehavior)
		!= nullptr) {
		//return;
		// TODO: react to not being able to take damage
	}
	currentHealth -= damage;
	// ripple near where damage was dealt
	// with a limit of ten
	if (ripples.size() == MAX_RIPPLE_COUNT) {
		ripples.pop_front();
	}
	ripples.push_back(RippleData(currentFrameTime,
		glm::vec3(surfacePointLocal[0], surfacePointLocal[1],
			surfacePointLocal[2])));
	if (currentHealth < 0) {
		currentHealth = 0;
	}
	std::cout << "Current health after taking damage: " << currentHealth << std::endl;
}

GameObjectBehavior::BehaviorStatus MothershipBehavior::UpdateStateMachine(
	float time, float deltaTime) {
	currentFrameTime = time;
	RemoveOldRipples();
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

void MothershipBehavior::RemoveOldRipples() {
	if (ripples.size() == 0) {
		return;
	}
	// remove old ripples, if any
	RippleData topmostRipple = ripples.front();
	while (topmostRipple.timeCreated + maxRippleDurationSeconds <
		currentFrameTime) {
		ripples.pop_front();
		if (ripples.size() == 0) {
			break;
		}
		topmostRipple = ripples.front();
	}
}

void* MothershipBehavior::GetUniformBufferModelViewProjRipple(
	size_t& uboSize, VkExtent2D const& swapChainExtent,
	const glm::mat4& viewMatrix,
	float time,
	float deltaTime) {
	UniformBufferObjectModelViewProjRipple* ubo =
		new UniformBufferObjectModelViewProjRipple();
	ubo->model = GetModelMatrix();
	ubo->view = viewMatrix;
	ubo->proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
		(float)swapChainExtent.height, 0.1f, 1000.0f);
	ubo->proj[1][1] *= -1; // flip Y -- opposite of opengl
	ubo->time = time;

	ubo->time = currentFrameTime;
	ubo->maxRippleDuration = maxRippleDurationSeconds;
	size_t numCurrentRipples = ripples.size();
	for (size_t i = 0; i < numCurrentRipples; i++) {
		auto& currentRipple = ripples[i];
		//ubo->ripplePointsLocal[i] = currentRipple.position;
		ubo->rippleStartTime = currentRipple.timeCreated; // TODO: fix
	}
	// disable any old ripples
	if (numCurrentRipples < 1) { // TODO: fix
		int difference = 1 - numCurrentRipples;
		for (size_t i = numCurrentRipples; i < numCurrentRipples + difference;
			i++) {
			ubo->rippleStartTime = -1.0f; // TODO: fix
		}
	}

	uboSize = sizeof(*ubo);
	return ubo;
}

