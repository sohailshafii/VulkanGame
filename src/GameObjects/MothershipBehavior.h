#pragma once

#include "GameObjectBehavior.h"
#include "ShipStateBehavior.h"
#include <glm/glm.hpp>
#include <memory>

class GameObject;

class MothershipBehavior : public GameObjectBehavior
{
public:
	typedef void (*SpawnedPawnDelegate) (
		std::shared_ptr<GameObject>const & );

	MothershipBehavior(Scene * const scene);
	MothershipBehavior();
	~MothershipBehavior();

	virtual void UpdateSelf(float time, float deltaTime) override;

	void RegisterSpawnedPawnSubscriber(SpawnedPawnDelegate Subscriber);
	
	void ClearSpawnedPawnSubscribers();

	void SpawnPawnObject() const;

private:
	ShipStateBehavior *currentShipStateBehavior;

	glm::vec3 axisOfRotation;
	SpawnedPawnDelegate onSpawnedPawnSubscriber;

	void Initialize();
	void UpdateStateMachine(float time, float deltaTime);
};
