#pragma once

#include "GameObjectBehavior.h"
#include "Camera.h"

// TODO: manage health
class PlayerGameObjectBehavior : public GameObjectBehavior
{
public:
	PlayerGameObjectBehavior(Scene * const scene,
		std::shared_ptr<Camera> const & playerCamera)
		: GameObjectBehavior(scene), playerCamera(playerCamera)
	{

	}

	PlayerGameObjectBehavior(std::shared_ptr<Camera> const& playerCamera)
		: GameObjectBehavior(), playerCamera(playerCamera)
	{
	}
	
	~PlayerGameObjectBehavior()
	{

	}

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
	std::shared_ptr<Camera> playerCamera;
};
