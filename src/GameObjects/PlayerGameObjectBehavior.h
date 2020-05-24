#pragma once

#include "GameObjectBehavior.h"

class PlayerGameObjectBehavior : public GameObjectBehavior
{
public:
	PlayerGameObjectBehavior()
	{

	}
	
	~PlayerGameObjectBehavior()
	{

	}

	virtual void UpdateSelf(float time, float deltaTime) override;

private:
};
