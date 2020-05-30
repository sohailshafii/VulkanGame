#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>

class MothershipBehavior : public GameObjectBehavior
{
public:
	MothershipBehavior()
	{
		axisOfRotation = glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f));
	}
	
	~MothershipBehavior()
	{

	}

	virtual void UpdateSelf(float time, float deltaTime) override;

private:
	glm::vec3 axisOfRotation;
};