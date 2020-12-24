#include "FontGameObjectBehavior.h"
#include "DescriptorSetFunctions.h"

GameObjectBehavior::BehaviorStatus FontGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	return GameObjectBehavior::BehaviorStatus::Normal;
}

// TODO: right now font knows its material is font, but that might not always be the case!
// how to relate the two?
void* FontGameObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = color;

	uboSize = sizeof(*ubo);
	return ubo;
}
