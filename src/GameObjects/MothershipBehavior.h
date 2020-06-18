#pragma once

#include "GameObjectBehavior.h"
#include <glm/glm.hpp>
#include <memory>

class GameObject;

class MothershipBehavior : public GameObjectBehavior
{
public:
	typedef void (*SpawnedGameObjectDelegate) (std::shared_ptr<GameObject>const & );

	MothershipBehavior()
	{
		axisOfRotation = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
	~MothershipBehavior()
	{

	}

	virtual void UpdateSelf(float time, float deltaTime) override;

	void RegisterSpawnedGameObjectSubscriber(SpawnedGameObjectDelegate Subscriber);
	
	void ClearSpawnedGameObjectSubscribers();

private:
	glm::vec3 axisOfRotation;
	SpawnedGameObjectDelegate OnSpawnedGameObjectSubscriber;


	enum ShipState {
		Idle = 0,
		FiringPawnsLevel1 = 0,
		FiringPawnsLevel2 = 0,
		FiringPawnsLevel3 = 0
	};
};
