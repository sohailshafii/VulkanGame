
#include "MenuObject.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Resources/Model.h"
#include "Resources/Material.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/FontObjects/FontGameObjectBehavior.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/FontObjects/FontTextureBuffer.h"

MenuObject::MenuObject(std::string const& menuText,
	FontTextureBuffer* fontTextureBuffer,
	std::shared_ptr<Material>& gameObjectMaterial,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	this->gameObjectMaterial = gameObjectMaterial;

	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 5.0f, 100.0f));
	localToWorldTransform = glm::scale(localToWorldTransform,
		glm::vec3(10.0f, 10.0f, 1.0f));

	std::shared_ptr<Model> menuModel = Model::CreateQuad(
		glm::vec3(-0.5f, -0.5f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	
	float advanceVal;
	for (unsigned char character : menuText) {
		// TODO: modify texture coords to access letter in text texture
		auto newGameObject = GameObjectCreator::CreateGameObject(
			this->gameObjectMaterial, CreateModelForCharacter(
				character,
				menuModel.get(),
				fontTextureBuffer,
				advanceVal),
			std::make_unique<FontGameObjectBehavior>(
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
		textGameObjects.push_back(newGameObject);
	}
}

/// <summary>
/// Create a model for the character based on the reference
/// model passed in.
/// </summary>
std::shared_ptr<Model> MenuObject::CreateModelForCharacter(
	unsigned char character, Model const * model,
	FontTextureBuffer* fontTextureBuffer,
	float &advanceVal) {
	std::shared_ptr<Model> duplicateModel =
		std::make_shared<Model>();
	*duplicateModel = *model;

	auto& positioningInfo = fontTextureBuffer->GetPositioningInfo(character);
	auto& modelVerts = duplicateModel->GetVertices();
	for (size_t i = 0; i < modelVerts.size(); i++) {
		auto modifiedPos = modelVerts[i].position;
		modifiedPos.x += positioningInfo.bitMapLeft + advanceVal;
		modifiedPos.y -= (positioningInfo.rows - positioningInfo.bitMapTop);
	}

	// each advance is 64 pixels
	advanceVal += positioningInfo.advanceX >> 6;

	return duplicateModel;
}
