#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>
#include <memory>

class GameObject;

class MothershipBehavior : public GameObjectBehavior
{
public:
	typedef void (*SpawnedGameObjectDelegate) (std::shared_ptr<GameObject>const & );

	MothershipBehavior();

	virtual void UpdateSelf(float time, float deltaTime) override;

	void RegisterSpawnedGameObjectSubscriber(SpawnedGameObjectDelegate Subscriber);
	
	void ClearSpawnedGameObjectSubscribers();

private:
	enum class ShipState {
		Idle,
		FiringPawnsLevel1,
		FiringPawnsLevel2,
		FiringPawnsLevel3
	};

	ShipState currentShipState;


	glm::vec3 axisOfRotation;
	SpawnedGameObjectDelegate onSpawnedGameObjectSubscriber;

	void UpdateStateMachine(float time, float deltaTime);

	ShipState UpdateIdleAndGetNextState(float time, float deltaTime);
	ShipState UpdateLevel1AndGetNextState(float time, float deltaTime);
	ShipState UpdateLevel2AndGetNextState(float time, float deltaTime);
	ShipState UpdateLevel3AndGetNextState(float time, float deltaTime);
};
