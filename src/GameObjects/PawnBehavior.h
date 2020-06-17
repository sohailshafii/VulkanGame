#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>

class PawnBehavior : public GameObjectBehavior
{
public:
	PawnBehavior();
	
	~PawnBehavior();

	virtual void UpdateSelf(float time, float deltaTime) override;

private:
	
};
