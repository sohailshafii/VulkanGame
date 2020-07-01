#pragma once

#include "GameObjectBehavior.h"

class PlayerGameObjectBehavior : public GameObjectBehavior
{
public:
	PlayerGameObjectBehavior(Scene * const scene)
		: GameObjectBehavior(scene)
	{

	}

	PlayerGameObjectBehavior()
		: GameObjectBehavior()
	{
	}
	
	~PlayerGameObjectBehavior()
	{

	}

	virtual void UpdateSelf(float time, float deltaTime) override;

private:
};
