#include "BulletBehavior.h"
#include "GameObject.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

const float BulletBehavior::acceleration = 0.2f;
const float BulletBehavior::maxVelocityMagnitude = 4.0f;

BulletBehavior::BulletBehavior() : currentVelocity(0.0f) {
}

BulletBehavior::BulletBehavior(Scene* const scene) 
	: GameObjectBehavior(scene), currentVelocity(0.0f) {
	int breakVar;
	breakVar = 1;
}
	
BulletBehavior::~BulletBehavior() {
}

GameObjectBehavior::BehaviorStatus BulletBehavior::UpdateSelf(float time,
	float deltaTime) {
	if (scene == nullptr) {
		return GameObjectBehavior::BehaviorStatus::Normal;
	}

	return GameObjectBehavior::BehaviorStatus::Normal;
}
