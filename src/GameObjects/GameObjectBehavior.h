#pragma once

class GameObjectBehavior
{
public:
	GameObjectBehavior()
	{

	}
	
	virtual ~GameObjectBehavior()
	{

	}

	virtual void UpdateSelf() = 0;

private:
};
