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

	virtual void GetUBOInformation(void** uboData, size_t& uboSize) override;

private:
	static const int maxHealth;

	ShipStateBehavior *currentShipStateBehavior;
	float radius;
	int currentHealth;
	std::stack<glm::vec3> ripplePositions;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time, float deltaTime);
};
