#pragma once

#include "GameObjects/GameObjectBehavior.h"

class BasicTurret;

class BasicTurretBehavior : public GameObjectBehavior {
public:
	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) override;
	
	void SetTurret(BasicTurret* basicTurret) {
		turret = basicTurret;
	}

private:
	// don't own
	BasicTurret* turret;
};
