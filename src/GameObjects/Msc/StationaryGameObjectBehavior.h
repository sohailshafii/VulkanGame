#pragma once

#include "GameObjectBehavior.h"

class StationaryGameObjectBehavior : public GameObjectBehavior
{
public:
	StationaryGameObjectBehavior(Scene * const scene)
		: GameObjectBehavior(scene)
	{

	}

	StationaryGameObjectBehavior()
		: GameObjectBehavior()
	{
	}
	
	~StationaryGameObjectBehavior()
	{

	}

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
};
