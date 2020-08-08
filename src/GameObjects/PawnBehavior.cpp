#include "PawnBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

PawnBehavior::PawnBehavior() {
}

PawnBehavior::PawnBehavior(Scene* const scene) 
	: GameObjectBehavior(scene) {
}
	
PawnBehavior::~PawnBehavior() {
	
}

void PawnBehavior::UpdateSelf(float time, float deltaTime) {
	if (scene != nullptr) {
		auto playerGameObject = scene->GetPlayerGameObject();
		if (playerGameObject != nullptr) {
			// TODO
		}
	}
}
