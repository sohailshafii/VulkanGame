#pragma once

#include "GameObjects/GameObjectBehavior.h"
#include <random>

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
	std::mt19937 mt;
	std::uniform_real_distribution<float> distX, distY, distZ;

	float idleTransitionTime;

	void Shoot();
	void Cooldown();
	void Idle(float currentTime);
};
