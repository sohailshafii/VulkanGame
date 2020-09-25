#include "PlayerGameObjectBehavior.h"

GameObjectBehavior::BehaviorStatus PlayerGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	this->SetWorldPosition(playerCamera->GetWorldPosition());
	return GameObjectBehavior::Normal;
}
