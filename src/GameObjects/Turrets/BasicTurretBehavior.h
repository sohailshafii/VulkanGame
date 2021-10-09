#pragma once

#include "GameObjects/GameObjectBehavior.h"

class BasicTurret;

class BasicTurretBehavior : public GameObjectBehavior {
public:
	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) override;

private:
	// don't own
	BasicTurret* turret;
};
