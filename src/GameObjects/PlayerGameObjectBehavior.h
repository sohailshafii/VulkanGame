#pragma once

#include "GameObjectBehavior.h"
#include "Camera.h"

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

	virtual void GetUBOInformation(void** uboData, size_t& uboSize) override;

private:
	std::shared_ptr<Camera> playerCamera;
};
