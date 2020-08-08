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

	virtual void UpdateSelf(float time, float deltaTime) override;

private:
	static const float acceleration;
	static const float maxVelocityMagnitude;

	float currentVelocity;
};
