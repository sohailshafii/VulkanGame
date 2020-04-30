#pragma once

#include "GameObjectBehavior.h"

class AIGameObjectBehavior : public GameObjectBehavior
{
public:
	AIGameObjectBehavior()
	{

	}
	
	~AIGameObjectBehavior()
	{

	}

	virtual void UpdateSelf() override;

private:
};
