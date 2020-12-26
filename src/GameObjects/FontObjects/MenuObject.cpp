
#include "MenuObject.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Resources/Model.h"
#include "Resources/Material.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/FontObjects/FontGameObjectBehavior.h"
#include "GameObjects/GameObject.h"

MenuObject::MenuObject(std::string const& menuText,
	std::shared_ptr<Model>& model,
	std::shared_ptr<Material>& gameObjectMaterial,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	this->gameObjectMaterial = gameObjectMaterial;

	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 5.0f, 100.0f));
	localToWorldTransform = glm::scale(localToWorldTransform,
		glm::vec3(10.0f, 10.0f, 1.0f));
	
	for (char character : menuText) {
		// TODO: modify texture coords to access letter in text texture
		auto newGameObject = GameObjectCreator::CreateGameObject(
			this->gameObjectMaterial, model,
			std::make_unique<FontGameObjectBehavior>(
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
		textGameObjects.push_back(newGameObject);
	}
}

std::shared_ptr<Model> CreateModelForCharacter(
	std::shared_ptr<Model> const & model) {
	return nullptr;
}
