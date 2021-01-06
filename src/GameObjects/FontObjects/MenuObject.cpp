
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
	
	float advanceVal = 0.0f;
	for (unsigned char character : menuText) {
		auto newGameObject = GameObjectCreator::CreateGameObject(
			this->gameObjectMaterial, CreateModelForCharacter(
				character,
				fontTextureBuffer,
				advanceVal,
				0.1f),
			std::make_unique<FontGameObjectBehavior>(
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
		textGameObjects.push_back(newGameObject);
	}
}

/// <summary>
/// Create a model for the character based on the reference
/// model passed in. Since all characters share a common
/// local-to-world transformation, we affect their local
/// coordinates so that its position in menu object is appropriate.
/// If the object is "Test," affect the local coordinates of "e"
/// so that it coems after "T" as both have the same local-to-world
/// transform.
/// </summary>
std::shared_ptr<Model> MenuObject::CreateModelForCharacter(
	unsigned char character, FontTextureBuffer* fontTextureBuffer,
	float &advanceVal,
	float scale) {

	auto& positioningInfo = fontTextureBuffer->GetPositioningInfo(character);
	float originX = positioningInfo.bitMapLeft * scale + advanceVal;
	float offsetY =-(positioningInfo.rows - positioningInfo.bitMapTop) * scale;
	if (character == 'y') {
		int breakVar;
		breakVar = 1;
	}
	std::shared_ptr<Model> characterModel = Model::CreateQuad(
			glm::vec3(originX, offsetY, 0.0f),
			glm::vec3((float)positioningInfo.width * scale, 0.0f, 0.0f),
			glm::vec3(0.0f, (float)positioningInfo.rows * scale, 0.0f));
	auto& modelVerts = characterModel->GetVertices();

	float textureCoordsBegin[2] = {
		positioningInfo.textureCoordsBegin[0],
		positioningInfo.textureCoordsBegin[1]
	};
	float textureCoordsEnd[2] = {
		positioningInfo.textureCoordsEnd[0],
		positioningInfo.textureCoordsEnd[1]
	};

	// go through and modify texture coordinates
	// there should be four verts total here
	modelVerts[0].texCoord[0] = textureCoordsBegin[0];
	modelVerts[0].texCoord[1] = textureCoordsEnd[1];

	modelVerts[1].texCoord[0] = textureCoordsEnd[0];
	modelVerts[1].texCoord[1] = textureCoordsEnd[1];

	modelVerts[2].texCoord[0] = textureCoordsBegin[0];
	modelVerts[2].texCoord[1] = textureCoordsBegin[1];

	modelVerts[3].texCoord[0] = textureCoordsEnd[0];
	modelVerts[3].texCoord[1] = textureCoordsBegin[1];

	// each advance is 64 pixels
	advanceVal += (positioningInfo.advanceX >> 6)* scale;

	return characterModel;
}
