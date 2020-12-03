
#include "MenuOption.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Resources/Model.h"

std::shared_ptr<Model> MenuObject::model;

MenuObject::MenuObject(std::string const& menuText,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	if (model.get() == nullptr) {
		model = Model::CreateQuad(glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
	}
}
