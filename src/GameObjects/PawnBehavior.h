#pragma once

#include "GameObjectBehavior.h"
#include "SceneManagement/Scene.h"
#include <glm/glm.hpp>

class PawnBehavior : public GameObjectBehavior
{
public:
	PawnBehavior();
	PawnBehavior(Scene* const scene);
	
	~PawnBehavior();

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
	static const float acceleration;
	static const float maxVelocityMagnitude;

	float currentVelocity;
	bool destroyed;
	bool initialized;
	glm::vec3 initialVectorToPlayer;
};
