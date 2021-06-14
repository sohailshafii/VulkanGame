#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>

class MenuSelectorObjectBehavior : public GameObjectBehavior
{
public:
	MenuSelectorObjectBehavior(Scene * scene,
		glm::vec4 const & color)
		: GameObjectBehavior(scene), color(color)
	{
	}

	MenuSelectorObjectBehavior(glm::vec4 const &  color)
		: GameObjectBehavior(), color(color)
	{
	}
	
	~MenuSelectorObjectBehavior()
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

	virtual void* CreateFragUBOData(size_t& uboSize) override;

private:
	glm::vec4 color;

protected:
	virtual void UpdateFBOUniformBufferColor(void* uboVoid) override;
};
