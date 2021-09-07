#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>

class MenuSelectorObjectBehavior : public GameObjectBehavior
{
public:
	MenuSelectorObjectBehavior(Scene * scene,
		glm::vec4 const & color)
		: GameObjectBehavior(scene)
	{
	}
	
	~MenuSelectorObjectBehavior()
	{
	}

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
};
