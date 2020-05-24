#pragma once

#include <glm/glm.hpp>

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
	
	void SetModelMatrix(glm::mat4 const & modelMatrix) {
		this->modelMatrix = modelMatrix;
	}
	
	glm::mat4 const & GetModelMatrix() {
		return modelMatrix;
	}

private:
	glm::mat4 modelMatrix;
};
