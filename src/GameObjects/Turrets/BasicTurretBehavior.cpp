
#include "GameObjects/Turrets/BasicTurretBehavior.h"
#include "GameObjects/Turrets/BasicTurret.h"
#include "SceneManagement/Scene.h"

const int BasicTurretBehavior::maxHealth = 200;

BasicTurretBehavior::BasicTurretBehavior(Scene* scene)
	: GameObjectBehavior(scene) {
	currentTurretState = TurretState::Idling;
	currentHealth = maxHealth;
}

GameObjectBehavior::BehaviorStatus BasicTurretBehavior::UpdateSelf(float time,
	float deltaTime) {
	auto currentBehaviorState = GameObjectBehavior::BehaviorStatus::Normal;

	switch (currentTurretState) {
		case TurretState::Shoot:
			Shoot();
			break;
		case TurretState::Cooldown:
			Cooldown();
			break;
		case TurretState::Dead:
			currentBehaviorState = GameObjectBehavior::BehaviorStatus::Destroyed;
			break;
		default:
			Idle();
			break;
	}

	return currentBehaviorState;
}

void BasicTurretBehavior::TakeDamage() {
	// TODO
}

void BasicTurretBehavior::Shoot() {
	// TODO
}

void BasicTurretBehavior::Cooldown() {
	// TODO
}

void BasicTurretBehavior::Idle() {
	// TODO
}
