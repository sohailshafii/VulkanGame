#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include "GameObjectCreationUtilFuncs.h"
#include "DescriptorSetFunctions.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

const int MothershipBehavior::maxHealth = 300;
const float MothershipBehavior::maxRippleDurationSeconds = 2.0f;
const float MothershipBehavior::maxStalkDurationSeconds = 1.0f;
const float MothershipBehavior::maxShudderDurationSeconds = 0.25f;

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
	return UpdateStateMachine(time, deltaTime);
}

void MothershipBehavior::SpawnPawn() {
	if (scene != nullptr) {
		float randPhi = 3.14f * 0.5f * ((float)rand()/RAND_MAX);
		float randTheta = 3.14f * 2.0f * ((float)rand() / RAND_MAX);
		float adjustedRadius = radius * 0.7f;
		glm::vec3 randomPos(adjustedRadius * sin(randTheta) * sin(randPhi),
			adjustedRadius * cos(randPhi),
			adjustedRadius * cos(randTheta) * sin(randPhi));
		scene->SpawnGameObject(Scene::SpawnType::Pawn, randomPos,
			glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 worldToModelMat = glm::inverse(modelMatrix);
		glm::vec4 surfacePointLocal = worldToModelMat * glm::vec4(randomPos, 1.0f);
		AddNewStalk(surfacePointLocal);
	}
}

void MothershipBehavior::Initialize() {
	currentShipStateBehavior = new ShipIdleStateBehavior();
}

bool MothershipBehavior::TakeDamageIfHit(int damage, glm::vec3 const& possibleHitPosition) {
	if (currentHealth == 0) {
		return false;
	}
	
	glm::vec3 worldPosition = GetWorldPosition();
	glm::vec3 vectorFromCenter = possibleHitPosition - worldPosition;
	float vecFromCenterMagn = glm::length(vectorFromCenter);
	if (vecFromCenterMagn > radius) {
		return false;
	}

	// shudder if we can't take damage
	if (dynamic_cast<ShipIdleStateBehavior*>(currentShipStateBehavior)
		!= nullptr) {
		shudderStartTime = currentFrameTime;
		return false;
	}
	// reset shudder time once the last one has elapsed
	else if (currentFrameTime > shudderStartTime + maxShudderDurationSeconds){
		shudderStartTime = -1.0f;
	}
	currentHealth -= damage;

	vectorFromCenter /= vecFromCenterMagn;
	glm::vec3 surfacePoint = worldPosition + vectorFromCenter * radius;
	glm::mat4 worldToModelMat = glm::inverse(modelMatrix);
	glm::vec4 surfacePointLocal = worldToModelMat * glm::vec4(surfacePoint, 1.0f);

	if (currentHealth < 0) {
		currentHealth = 0;
	}
	AddNewRipple(surfacePointLocal);
	std::cout << "Current health after taking damage: " << currentHealth << std::endl;
	return true;
}

void MothershipBehavior::AddNewRipple(glm::vec4 const & surfacePointLocal) {
	// ripple near where damage was dealt
	// with a limit of ten
	if (ripples.size() == MAX_RIPPLE_COUNT) {
		ripples.pop_front();
	}
	ripples.push_back(RippleData(currentFrameTime,
		MothershipBehavior::maxRippleDurationSeconds * 0.33f +
		((float)rand() / RAND_MAX) * MothershipBehavior::maxRippleDurationSeconds * 0.67f,
		glm::vec3(surfacePointLocal[0], surfacePointLocal[1], surfacePointLocal[2]))
	);
}

void MothershipBehavior::AddNewStalk(glm::vec4 const& surfacePointLocal) {
	if (stalks.size() == MAX_STALK_COUNT) {
		stalks.pop_front();
	}

	stalks.push_back(StalkData(
		glm::vec3(surfacePointLocal[0], surfacePointLocal[1], surfacePointLocal[2]),
		currentFrameTime));
}

GameObjectBehavior::BehaviorStatus MothershipBehavior::UpdateStateMachine(
	float time, float deltaTime) {
	currentFrameTime = time;
	RemoveOldRipples();
	RemoveOldStalks();
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

void MothershipBehavior::RemoveOldStalks() {
	if (stalks.size() == 0) {
		return;
	}

	StalkData topmostStalk = stalks.front();
	while (topmostStalk.timeCreated + maxStalkDurationSeconds <
		currentFrameTime) {
		stalks.pop_front();
		if (stalks.size() == 0) {
			break;
		}
		topmostStalk = stalks.front();
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

	ubo->shudderDuration = maxShudderDurationSeconds;
	ubo->shudderStartTime = shudderStartTime;

	UpdateUBORippleData(ubo);
	UpdateUBOStalkData(ubo);

	uboSize = sizeof(*ubo);
	return ubo;
}

void MothershipBehavior::UpdateUBORippleData(
	UniformBufferObjectModelViewProjRipple* ubo) {
	size_t numCurrentRipples = ripples.size();
	for (size_t i = 0; i < numCurrentRipples; i++) {
		auto& currentRipple = ripples[i];
		RipplePointLocal& ripplePointLocal = ubo->ripplePointsLocal[i];
		ripplePointLocal.ripplePosition = glm::vec4(currentRipple.position,
			1.0f);
		ripplePointLocal.rippleDuration = currentRipple.duration;
		ripplePointLocal.rippleStartTime = currentRipple.timeCreated;
	}
	// disable any old ripples
	if (numCurrentRipples < MAX_RIPPLE_COUNT) {
		int difference = MAX_RIPPLE_COUNT - numCurrentRipples;
		for (size_t i = numCurrentRipples; i < numCurrentRipples + difference;
			i++) {
			RipplePointLocal& ripplePointLocal = ubo->ripplePointsLocal[i];
			ripplePointLocal.rippleStartTime = -1.0f;
		}
	}
}

void MothershipBehavior::UpdateUBOStalkData(
	UniformBufferObjectModelViewProjRipple* ubo) {
	size_t numCurrentStalks = stalks.size();
	for (size_t i = 0; i < numCurrentStalks; i++) {
		auto& currentStalk = stalks[i];
		StalkPointLocal& stalkPointLocal = ubo->stalkPointsLocal[i];
		stalkPointLocal.stalkPosition = glm::vec4(currentStalk.position,
			1.0f);
		stalkPointLocal.stalkSpawnTime = currentStalk.timeCreated;
	}
	// disable any old ripples
	if (numCurrentStalks < MAX_STALK_COUNT) {
		int difference = MAX_STALK_COUNT - numCurrentStalks;
		for (size_t i = numCurrentStalks; i < numCurrentStalks + difference;
			i++) {
			StalkPointLocal& stalkPointLocal = ubo->stalkPointsLocal[i];
			stalkPointLocal.stalkSpawnTime = -1.0f;
		}
	}
}

