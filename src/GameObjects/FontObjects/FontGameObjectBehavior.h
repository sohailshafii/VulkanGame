#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>

class FontGameObjectBehavior : public GameObjectBehavior
{
public:
	FontGameObjectBehavior(Scene * scene,
		glm::vec4 const & color)
		: GameObjectBehavior(scene), color(color)
	{
	}

	FontGameObjectBehavior(glm::vec4 const &  color)
		: GameObjectBehavior(), color(color)
	{
	}
	
	~FontGameObjectBehavior()
	{

	}

	glm::vec4 GetColor() const {
		return color;
	}

	void SetColor(glm::vec4 const& color) {
		this->color = color;
	}

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
	glm::vec4 color;
};
