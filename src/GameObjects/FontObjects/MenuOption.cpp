
#include "MenuOption.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Resources/Model.h"
#include "Resources/Material.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Msc/StationaryGameObjectBehavior.h"
#include "GameObjects/GameObject.h"

std::shared_ptr<Model> MenuObject::model;
std::shared_ptr<Material> MenuObject::gameObjectMaterial;

MenuObject::MenuObject(std::string const& menuText,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	if (MenuObject::model.get() == nullptr) {
		MenuObject::model = Model::CreateQuad(glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
	}

	if (MenuObject::gameObjectMaterial == nullptr) {
		// TODO: procedural create texture
		MenuObject::gameObjectMaterial = GameObjectCreator::CreateMaterial(
			DescriptorSetFunctions::MaterialType::Text,
			"texture.jpg", resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	}

	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 0.0f, 4.0f));
	for (char character : menuText) {
		// TODO: modify texture coords to access letter in text texture
		auto newGameObject = GameObjectCreator::CreateGameObject(gameObjectMaterial,
			MenuObject::model,
			std::make_unique<StationaryGameObjectBehavior>(),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	}
}
