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
	void ComputeHeadingDir(std::shared_ptr<GameObject>
						   const & playerGameObject);
	
	enum PawnState { JustCreated = 0, Spawning, HeadingToPlayer, Destroyed };
	// TODO states for pawn
	//glm::vec3 ComputeCurrentPawnPosition(std::shared_ptr<GameObject>
		//								 const & playerGameObject);
	
	static const float acceleration;
	static const float maxVelocityMagnitude;

	float currentVelocity;
	PawnState currentPawnState;
	glm::vec3 initialVectorToPlayer;
	glm::vec3 initialForwardVec;
};
