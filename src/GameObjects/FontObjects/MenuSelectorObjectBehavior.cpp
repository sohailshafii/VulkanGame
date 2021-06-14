#include "MenuSelectorObjectBehavior.h"
#include "DescriptorSetFunctions.h"
#include "Common.h"

GameObjectBehavior::BehaviorStatus MenuSelectorObjectBehavior::UpdateSelf(
	float time, float deltaTime) {
	return GameObjectBehavior::BehaviorStatus::Normal;
}

void* MenuSelectorObjectBehavior::CreateFragUBOData(size_t& uboSize) {
	UniformBufferUnlitColor* ubo =
		new UniformBufferUnlitColor();
	ubo->objectColor = color;

	uboSize = sizeof(*ubo);
	return ubo;
}

void MenuSelectorObjectBehavior::UpdateFBOUniformBufferColor(void* uboVoid) {
	UniformBufferUnlitColor* ubo =
		(UniformBufferUnlitColor*)uboVoid;
	ubo->objectColor = color;
}

