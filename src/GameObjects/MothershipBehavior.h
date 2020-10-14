#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
#include <glm/glm.hpp>
#include <memory>
#include <deque>

class GameObject;

class MothershipBehavior : public GameObjectBehavior
{
public:
	MothershipBehavior(Scene * const scene, float radius);
	MothershipBehavior();
	~MothershipBehavior();

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

	void SpawnGameObject() const;

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

	static const int maxHealth;
	static const float maxRippleDurationSeconds;
	static const float maxShudderDurationSeconds;

	ShipStateBehavior *currentShipStateBehavior;
	float radius;
	int currentHealth;
	float currentFrameTime;
	std::deque<RippleData> ripples;
	float shudderStartTime;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time,
		float deltaTime);
	void RemoveOldRipples();

protected:
	virtual void* GetUniformBufferModelViewProjRipple(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime) override;
};
