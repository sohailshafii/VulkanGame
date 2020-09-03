#pragma once

#include "GameObjectBehavior.h"
#include "SceneManagement/Scene.h"
#include <glm/glm.hpp>

class BulletBehavior : public GameObjectBehavior
{
public:
	BulletBehavior();
	BulletBehavior(Scene* const scene);
	
	~BulletBehavior();

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
	static const float acceleration;
	static const float maxVelocityMagnitude;

	float currentVelocity;
};
