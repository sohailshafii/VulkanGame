#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
#include <glm/glm.hpp>
#include <memory>

class GameObject;

class MothershipBehavior : public GameObjectBehavior
{
public:
	// TODO: apply local-to-world scale to radius
	MothershipBehavior(Scene * const scene, float radius);
	MothershipBehavior();
	~MothershipBehavior();

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

	void SpawnGameObject() const;

private:
	ShipStateBehavior *currentShipStateBehavior;
	float radius;

	void Initialize();
	GameObjectBehavior::BehaviorStatus UpdateStateMachine(float time, float deltaTime);
};
