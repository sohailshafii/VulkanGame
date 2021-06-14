#include "FontGameObjectBehavior.h"
#include "DescriptorSetFunctions.h"
#include "Common.h"

GameObjectBehavior::BehaviorStatus FontGameObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	return GameObjectBehavior::BehaviorStatus::Normal;
}

// This assumes that our material is of a certain type (font shader)
// so the UBO return is based on that
void* FontGameObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = color;

	uboSize = sizeof(*ubo);
	return ubo;
}

void FontGameObjectBehavior::UpdateFBOUniformBufferColor(void* uboVoid) {
	UniformBufferUnlitColor* ubo =
		(UniformBufferUnlitColor*)uboVoid;
	ubo->objectColor = color;
}
