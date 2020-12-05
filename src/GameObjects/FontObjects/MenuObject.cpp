
#include "MenuObject.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Resources/Model.h"
#include "Resources/Material.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Msc/StationaryGameObjectBehavior.h"
#include "GameObjects/GameObject.h"

MenuObject::MenuObject(std::string const& menuText,
	std::shared_ptr<Model>& model,
	std::shared_ptr<Material>& gameObjectMaterial,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	this->model = model;
	this->gameObjectMaterial = gameObjectMaterial;

	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 0.0f, 4.0f));
	for (char character : menuText) {
		// TODO: modify texture coords to access letter in text texture
		auto newGameObject = GameObjectCreator::CreateGameObject(
			this->gameObjectMaterial, this->model,
			std::make_unique<StationaryGameObjectBehavior>(),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	}
}
