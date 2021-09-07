#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
#include <glm/glm.hpp>
#include <memory>
#include <deque>

class GameObject;
struct UniformBufferObjectModelViewProjRipple;

class MothershipBehavior : public GameObjectBehavior
{
public:
	MothershipBehavior(Scene * const scene, float radius);
	MothershipBehavior();
	~MothershipBehavior();

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

	void Reboot();

	void SpawnPawn();

	float GetRadius() const {
		return radius;
	}

	bool TakeDamageIfHit(int damage, glm::vec3 const& possibleHitPosition);

	int GetCurrentHealth() const {
		return currentHealth;
	}

	void UpdateUBOBehaviorData(UniformBufferObjectModelViewProjRipple* ubo);
	
	// this is a value that is matched against the value
	// in the mothership shader file. reference by pawn
	static const float stalkRiseDuration;

private:
	struct RippleData {
		RippleData(float timeCreated, float duration, glm::vec3 const& pos) {
			this->timeCreated = timeCreated;
			this->duration = duration;
			this->position = pos;
		}

		float timeCreated;
		float duration;
		glm::vec3 position;
	};

	struct StalkData {
		StalkData(glm::vec3 const & pos, float time) {
			this->position = pos;
			this->timeCreated = time;
		}

		glm::vec3 position;
		float timeCreated;
	};

	struct VertexColorModifierData {
		VertexColorModifierData(float timeCreated,
			float duration, float maxAngleRadians, glm::vec3 const& localPos,
			glm::vec3 const & desiredColor) {
			this->timeCreated = timeCreated;
			this->duration = duration;
			this->maxAngleRadians = maxAngleRadians;
			this->localPosition = localPos;

			this->desiredColor = desiredColor;
		}

		float timeCreated;
		float duration;
		float maxAngleRadians;
		glm::vec3 localPosition;
		glm::vec3 desiredColor;
	};

	static const int maxHealth;
	static const float maxRippleDurationSeconds;
	static const float maxStalkDurationSeconds;
	static const float maxShudderDurationSeconds;
	static const float maxDeathDurationSeconds;

	ShipStateBehavior *currentShipStateBehavior;
	float radius;
	int currentHealth;
	float currentFrameTime;
	std::deque<RippleData> ripples;
	std::deque<StalkData> stalks;
	std::deque<VertexColorModifierData> vertexColorModifiers;
	std::vector<glm::vec3> originalModelColors;
	float shudderStartTime;

	float deathStartTime;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time,
		float deltaTime);
	void AddVertexColorModifier(glm::vec3 const& localPosition,
		float radius, glm::vec3 const& color);
	void UpdateModelColorsBasedOnCurrentModifiers();
	void StoreOriginalColorsIfRequired();
	void RestoreOldColorsIfRequired();

	void AddNewRipple(glm::vec4 const& surfacePointLocal);
	void AddNewStalk(glm::vec4 const& surfacePointLocal);
	void RemoveOldRipples();
	void RemoveOldStalks();

	glm::vec3 SamplePositionOnPlane(glm::vec3 const& planePosition,
		glm::vec3 const& planeNormal, float maxRadius);
	glm::vec3 FindVectorPerpendicularToInputVec(glm::vec3 const& inputVector);
	bool RaySphereIntersection(glm::vec3 const& rayDir, glm::vec3 const& rayOrigin,
		float radius, glm::vec3 const& sphereOrigin, float& tVal);

	int FindIndexOfStalkCloseToPosition(glm::vec3 const& surfacePointLocal,
		float maxAngleRadians);
	float FindUnsignedAngleBetweenTwoLocalPoints(glm::vec3 const& surfacePointLocal1,
		glm::vec3 const& surfacePointLocal2);

	void UpdateUBORippleData(UniformBufferObjectModelViewProjRipple* ubo);
	void UpdateUBOStalkData(UniformBufferObjectModelViewProjRipple* ubo);

	void Die();
};
