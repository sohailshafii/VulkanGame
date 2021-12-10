
#include "GameObjects/Turrets/BasicTurretBehavior.h"
#include "GameObjects/Turrets/BasicTurret.h"
#include "SceneManagement/Scene.h"

const int BasicTurretBehavior::maxHealth = 200;

BasicTurretBehavior::BasicTurretBehavior(Scene* scene)
	: GameObjectBehavior(scene) {
	currentTurretState = TurretState::Idling;
	currentHealth = maxHealth;
	idleTransitionTime = -1.0f;

	std::random_device rd;
	mt = std::mt19937(rd());
	distX = std::uniform_real_distribution<float>(-1.0f, 1.0f);
	distY = std::uniform_real_distribution<float>(0.2f, 1.0f);
	distZ = std::uniform_real_distribution<float>(-1.0f, 1.0f);
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
			Idle(time);
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

void BasicTurretBehavior::Idle(float currentTime) {
	if (idleTransitionTime < -1.0f) {
		idleTransitionTime = currentTime + 2.0f;
	}

	if (idleTransitionTime < currentTime) {
		idleTransitionTime = currentTime + 2.0f;
		// TODO: debug slerp
		auto currLookAtPoint = turret->GetCurrentLookAtPoint();
		auto newPoint = glm::vec3(distX(mt), distY(mt), distZ(mt));
		while (glm::length(newPoint - currLookAtPoint) < 0.01f) {
			newPoint = glm::vec3(distX(mt), distY(mt), distZ(mt));
		}
		turret->SetGunLookRotation(newPoint, true);
	}
}
