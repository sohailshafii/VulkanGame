#pragma once

#include "GameObjectBehavior.h"
#include "SceneManagement/Scene.h"
#include <glm/glm.hpp>

class BulletBehavior : public GameObjectBehavior
{
public:
	BulletBehavior();
	BulletBehavior(Scene* const scene, glm::vec3 const & velocityVector);
	
	~BulletBehavior();

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
	static const float acceleration;
	static const float maxVelocityMagnitude;

	float currentVelocity;
	glm::vec3 velocityVector;
};
