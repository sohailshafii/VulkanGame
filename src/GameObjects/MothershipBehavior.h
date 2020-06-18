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
	
	~MothershipBehavior();

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

	ShipState CurrentShipState;
	glm::vec3 axisOfRotation;
	SpawnedGameObjectDelegate OnSpawnedGameObjectSubscriber;

	void UpdateStateMachine();
};
