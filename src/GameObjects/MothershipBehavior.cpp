#include "MothershipBehavior.h"
#include "ShipIdleStateBehavior.h"
#include "GameObjectCreationUtilFuncs.h"
#include "DescriptorSetFunctions.h"
#include "GameObject.h"
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
		auto playerGameObject = scene->GetPlayerGameObject();
		if (playerGameObject == nullptr) {
			return;
		}
		// create plane representing half space facing player
		// we want to sample point points along this plane. Then cast a vector
		// from that point to the player to get position on sphere.
		// the sphere position is valid assuming the path from its point
		// to the player does not hit the sphere again (i.e. doesn't
		// hit sphere from the inside)
		// TODO: implement
		auto playerWorldPosition = playerGameObject->GetWorldPosition();
		auto planePosition = GetWorldPosition();
		auto worldPosVecToPlayer = playerWorldPosition - planePosition;
		worldPosVecToPlayer = glm::normalize(worldPosVecToPlayer);
		float planeDistance =-glm::dot(planePosition, worldPosVecToPlayer);

		float randPhi = 3.14f * 0.5f * ((float)rand()/RAND_MAX);
		float randTheta = 3.14f * 2.0f * ((float)rand() / RAND_MAX);
		float reducedRadius = radius * 0.7f;
		glm::vec3 randomPos(reducedRadius * sin(randTheta) * sin(randPhi),
			reducedRadius * cos(randPhi),
			reducedRadius * cos(randTheta) * sin(randPhi));

		// on wrong side? reflect along normal
		float randomPosDist = glm::dot(worldPosVecToPlayer, randomPos)
			+ planeDistance;
		// TODO: this is wrong. need to fix
		std::cout << "*************Distance: " << randomPosDist << std::endl;
		if (randomPosDist < 0.0f) {
			// move by 2x distance to plane to move to other side of sphere
			randomPos += 2.0f * fabs(randomPosDist) * worldPosVecToPlayer;
			std::cout << "moving random pos to other side\n";
		}

		scene->SpawnGameObject(Scene::SpawnType::Pawn, randomPos,
			glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 worldToModelMat = glm::inverse(modelMatrix);
		glm::vec4 surfacePointLocal = worldToModelMat * glm::vec4(randomPos, 1.0f);
		AddNewStalk(surfacePointLocal);
	}
}

glm::vec3 MothershipBehavior::SamplePositionOnPlane(glm::vec3 const& planePosition,
	glm::vec3 const& planeNormal, float maxRadius) {
	glm::vec3 vectorOnPlane = FindVectorPerpendicularToInputVec(planeNormal);
	glm::vec3 vectorOnPlane2 = glm::cross(vectorOnPlane, planeNormal);
	glm::normalize(vectorOnPlane2);
	// make sure first vector is perpendicular to both
	vectorOnPlane = glm::cross(planeNormal, vectorOnPlane2);

	// now we have a coordinate system. time to sample
	float randAngle = 3.14f * 0.5f * ((float)rand() / RAND_MAX);
	return maxRadius * cos(randAngle) * vectorOnPlane +
		maxRadius * sin(randAngle) * vectorOnPlane2;
}

glm::vec3 MothershipBehavior::FindVectorPerpendicularToInputVec(
	glm::vec3 const& inputVector) {
	// find unit vector in coordinate axis that is perpendicular
		// to forward
	glm::vec3 candidate1(0.0f, 0.0f, 1.0f);
	glm::vec3 candidate2(0.0f, 1.0f, 0.0f);
	glm::vec3 candidate3(1.0f, 0.0f, 0.0f);
	float dot1 = fabs(glm::dot(candidate1, inputVector));
	float dot2 = fabs(glm::dot(candidate2, inputVector));
	float dot3 = fabs(glm::dot(candidate3, inputVector));
	glm::vec3 perpVec;
	// the most perpendicular one has the smallest dot product
	if (dot1 < dot2 && dot1 < dot3) {
		// subtract out the parallel part
		perpVec = candidate1 - inputVector * (candidate1 * inputVector);
	}
	else if (dot2 < dot1 && dot2 < dot3) {
		// subtract out the parallel part
		perpVec = candidate2 - inputVector * (candidate2 * inputVector);
	}
	else {
		// subtract out the parallel part
		perpVec = candidate3 - inputVector * (candidate3 * inputVector);
	}
	glm::normalize(perpVec);
	return perpVec;
}

bool MothershipBehavior::RaySphereIntersection(glm::vec3 const& rayDir,
	glm::vec3 const& rayOrigin, float radius, glm::vec3 const& sphereOrigin,
	float& tVal) {
	glm::vec3 centerToRayOrigin = rayOrigin - sphereOrigin;
	float a = glm::dot(rayDir, rayDir);
	float b = 2.0f * glm::dot(centerToRayOrigin, rayDir);
	float c = glm::dot(centerToRayOrigin, centerToRayOrigin) - radius* radius;
	float discr = b * b - 4.0f * a * c;

	if (discr < 0.0f) {
		return false;
	}

	float e = sqrt(discr);
	float denom = 2.0f * a;
	float t = (-b - e) / denom;
	// smaller root
	if (t > 0.0f) {
		tVal = t;
		return true;
	}

	t = (-b + e) / denom;
	if (t > 0.0f) {
		tVal = t;
		return true;
	}

	// all tests failed so far
	return false;
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

