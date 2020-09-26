#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
#include <glm/glm.hpp>
#include <memory>
#include <stack>

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

	void TakeDamage(int damage, glm::vec3 const& hitPosition);

	int GetCurrentHealth() const {
		return currentHealth;
	}

private:
	struct RippleData {
		RippleData(float timeCreated, glm::vec3 const& pos) {
			this->timeCreated = timeCreated;
			this->position = pos;
		}

		float timeCreated;
		glm::vec3 position;
	};

	static const int maxHealth;

	ShipStateBehavior *currentShipStateBehavior;
	float radius;
	int currentHealth;
	std::stack<RippleData> ripples;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time,
		float deltaTime);

protected:
	virtual void* GetUniformBufferModelViewProjRipple(
		size_t& uboSize, VkExtent2D const& swapChainExtent,
		const glm::mat4& viewMatrix,
		float time,
		float deltaTime) override;
};
