#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
#include <glm/glm.hpp>
#include <memory>

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

	void TakeDamage(int damage);

	int GetCurrentHealth() const {
		return currentHealth;
	}

private:
	static const int maxHealth;

	ShipStateBehavior *currentShipStateBehavior;
	float radius;
	int currentHealth;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time, float deltaTime);
};
