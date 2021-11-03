#pragma once

#include "GameObjects/GameObjectBehavior.h"

class BasicTurret;
class Scene;

class BasicTurretBehavior : public GameObjectBehavior {
public:
	BasicTurretBehavior(Scene* scene);

	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) override;
	
	void SetTurret(BasicTurret* basicTurret) {
		turret = basicTurret;
	}

	void TakeDamage();

private:
	enum class TurretState { Idling = 0, Shoot, Cooldown, Dead };

	// don't own
	BasicTurret* turret;

	TurretState currentTurretState;
	int currentHealth;

	static const int maxHealth;

	// TODO: need slerp in turret class
	int idleNewAzim, idleNewPolar;
	int idleTransitionTime;

	void Shoot();
	void Cooldown();
	void Idle();
};
