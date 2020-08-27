#pragma once

#include <glm/glm.hpp>
#include "SceneManagement/Scene.h"

class GameObjectBehavior
{
public:
	enum BehaviorStatus { Normal = 0, Destroyed };

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

	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) = 0;
	
	void MultiplyByMatrix(glm::mat4 const& newMatrix) {
		this->modelMatrix *= newMatrix;
	}

	void SetModelMatrix(glm::mat4 const & modelMatrix) {
		this->modelMatrix = modelMatrix;
	}
	
	glm::mat4 const & GetModelMatrix() {
		return modelMatrix;
	}

	void SetWorldPosition(glm::vec3 const& worldPos) {
		modelMatrix[3][0] = worldPos[0];
		modelMatrix[3][1] = worldPos[1];
		modelMatrix[3][2] = worldPos[2];
	}

	glm::vec3 GetWorldPosition() const {
		return glm::vec3(modelMatrix[3][0], modelMatrix[3][1],
			modelMatrix[3][2]);
	}

	void SetScene(Scene* scene) {
		this->scene = scene;
	}

protected:
	glm::mat4 modelMatrix;
	// we don't own this pointer; should be shared_ptr ideally
	Scene * scene;
};
