#pragma once

#include "GameObjectBehavior.h"
#include "SceneManagement/Scene.h"
#include <glm/glm.hpp>

class PawnBehavior : public GameObjectBehavior
{
public:
	PawnBehavior();
	PawnBehavior(Scene* const scene,
				 glm::vec3 const & initialForwardVec);
	
	~PawnBehavior();

	void Destroy() {
		currentPawnState = Destroyed;
	}

	virtual GameObjectBehavior::BehaviorStatus UpdateSelf(float time,
		float deltaTime) override;

private:
	bool IsCloseToPlayer(std::shared_ptr<GameObject>
						 const & playerGameObject,
						 glm::vec3 const & pawnPosition);
	glm::vec3 ComputeMovement(std::shared_ptr<GameObject>
						const & playerGameObject,
						float currentTime,
						float deltaTime);
	
	enum PawnState { JustCreated = 0, Spawning, HeadingToPlayer, Destroyed };
	
	static const float acceleration;
	static const float maxVelocityMagnitude;

	float currentVelocity;
	PawnState currentPawnState;
	glm::vec3 currentForwardVec;
	glm::vec3 startPosition;

	float timeBeginState, timeEndState;
};
