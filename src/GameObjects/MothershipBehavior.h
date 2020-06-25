#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
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
	ShipStateBehavior *currentShipStateBehavior;

	glm::vec3 axisOfRotation;
	SpawnedGameObjectDelegate onSpawnedGameObjectSubscriber;

	void UpdateStateMachine(float time, float deltaTime);
};
