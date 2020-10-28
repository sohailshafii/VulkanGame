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

	void SpawnPawn();

	float GetRadius() const {
		return radius;
	}

	bool TakeDamageIfHit(int damage, glm::vec3 const& possibleHitPosition);

	int GetCurrentHealth() const {
		return currentHealth;
	}

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

	static const int maxHealth;
	static const float maxRippleDurationSeconds;
	static const float maxStalkDurationSeconds;
	static const float maxShudderDurationSeconds;

	ShipStateBehavior *currentShipStateBehavior;
	float radius;
	int currentHealth;
	float currentFrameTime;
	std::deque<RippleData> ripples;
	std::deque<StalkData> stalks;
	float shudderStartTime;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time,
		float deltaTime);
	void AddNewRipple(glm::vec4 const& surfacePointLocal);
	void AddNewStalk(glm::vec4 const& surfacePointLocal);
	void RemoveOldRipples();
	void RemoveOldStalks();

protected:
	virtual void* GetUniformBufferModelViewProjRipple(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime) override;
	void UpdateUBORippleData(UniformBufferObjectModelViewProjRipple* ubo);
	void UpdateUBOStalkData(UniformBufferObjectModelViewProjRipple* ubo);

	glm::vec3 SamplePositionOnPlane(glm::vec3 const& planePosition,
		glm::vec3 const& planeNormal, float maxRadius);
	glm::vec3 FindVectorPerpendicularToInputVec(glm::vec3 const& inputVector);
	bool RaySphereIntersection(glm::vec3 const& rayDir, glm::vec3 const& rayOrigin,
		float radius, glm::vec3 const& sphereOrigin, float& tVal);
};
