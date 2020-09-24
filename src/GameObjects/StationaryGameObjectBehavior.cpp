#include "StationaryGameObjectBehavior.h"

GameObjectBehavior::BehaviorStatus StationaryGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	return GameObjectBehavior::Normal;
}

void StationaryGameObjectBehavior::GetUBOInformation(void** uboData,
	size_t& uboSize) {
	// TODO
}
