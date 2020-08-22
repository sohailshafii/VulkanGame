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
	
	void MultiplyByMatrix(glm::mat4 const& newMatrix) {
		this->modelMatrix *= newMatrix;
	}

	void SetModelMatrix(glm::mat4 const & modelMatrix) {
		this->modelMatrix = modelMatrix;
	}
	
	glm::mat4 const & GetModelMatrix() {
		return modelMatrix;
	}

	glm::vec3 GetWorldPosition() const {
		return glm::vec3(modelMatrix[3][0], modelMatrix[3][1],
			modelMatrix[3][3]);
	}

protected:
	glm::mat4 modelMatrix;
	// we don't own this pointer; should be shared_ptr ideally
	Scene * const scene;
};
