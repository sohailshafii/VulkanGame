#pragma once

#include "GameObjectBehavior.h"

class StationaryGameObjectBehavior : public GameObjectBehavior
{
public:
	StationaryGameObjectBehavior()
	{

	}
	
	~StationaryGameObjectBehavior()
	{

	}

	virtual void UpdateSelf(float time, float deltaTime) override;

private:
};
