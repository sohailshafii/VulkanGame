#include "PlayerGameObjectBehavior.h"

void PlayerGameObjectBehavior::UpdateSelf(float time, float deltaTime) {
	this->SetWorldPosition(playerCamera->GetWorldPosition());
}

