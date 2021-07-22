#include "PlayerGameObjectBehavior.h"

GameObjectBehavior::BehaviorStatus PlayerGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	this->SetRelativePosition(playerCamera->GetWorldPosition());
	return GameObjectBehavior::BehaviorStatus::Normal;
}
