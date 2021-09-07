#include "PlayerGameObjectBehavior.h"
#include "GameObjects/GameObject.h"

GameObjectBehavior::BehaviorStatus PlayerGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	this->GetGameObject()->SetLocalPosition(playerCamera->GetWorldPosition());
	return GameObjectBehavior::BehaviorStatus::Normal;
}
