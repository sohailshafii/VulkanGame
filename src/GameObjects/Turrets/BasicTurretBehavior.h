#pragma once

#include "GameObjects/GameObjectBehavior.h"

class BasicTurretBehavior : public GameObjectBehavior {
public:
	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) override;

private:
};
