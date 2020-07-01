#pragma once

#include <glm/glm.hpp>
#include "SceneManagement/Scene.h"

class GameObjectBehavior
{
public:
	GameObjectBehavior(Scene * const scene)
		: scene(scene)
	{

	}

	GameObjectBehavior()
		: scene(nullptr)
	{

	}
	
	virtual ~GameObjectBehavior()
	{

	}

	virtual void UpdateSelf(float time, float deltaTime) = 0;
	
	void SetModelMatrix(glm::mat4 const & modelMatrix) {
		this->modelMatrix = modelMatrix;
	}
	
	glm::mat4 const & GetModelMatrix() {
		return modelMatrix;
	}

protected:
	glm::mat4 modelMatrix;
	// we don't own this pointer; should be shared_ptr ideally
	Scene * const scene;
};
