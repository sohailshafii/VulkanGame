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

	virtual void UpdateSelf(float time, float deltaTime) override;

	void SpawnGameObject() const;

private:
	ShipStateBehavior *currentShipStateBehavior;
	float radius;

	void Initialize();
	void UpdateStateMachine(float time, float deltaTime);
};
