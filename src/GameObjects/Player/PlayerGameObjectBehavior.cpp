#include "PlayerGameObjectBehavior.h"
#include "GameObjects/GameObject.h"

GameObjectBehavior::BehaviorStatus PlayerGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	if (this->GetGameObject() != nullptr) {
		this->GetGameObject()->SetLocalPosition(playerCamera->GetWorldPosition());
	}
	return GameObjectBehavior::BehaviorStatus::Normal;
}
