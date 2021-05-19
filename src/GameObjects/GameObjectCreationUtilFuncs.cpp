
#include "GameObjectCreationUtilFuncs.h"
#include "Resources/ResourceLoader.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/GameObjectBehavior.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Model.h"

std::shared_ptr<GameObject> GameObjectCreator::CreateGameObject(
	std::shared_ptr<Material> const & material,
	std::shared_ptr<Model> const & gameObjectModel,
	std::shared_ptr<GameObjectBehavior> gameObjectBehavior,
	glm::mat4 const & localToWorldTransform,
	ResourceLoader* resourceLoader,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
	std::shared_ptr<GameObject> constructedGameObject =
		std::make_shared<GameObject>(gameObjectModel, material,
			gameObjectBehavior,
			gfxDeviceManager,
			logicalDeviceManager, commandPool);
	constructedGameObject->SetModelTransform(localToWorldTransform);

	return constructedGameObject;
}

std::shared_ptr<Material> GameObjectCreator::CreateMaterial(
	DescriptorSetFunctions::MaterialType materialEnumType,
	std::string const & mainTextureName,
	nlohmann::json const& materialNode,
	bool isRawTexture,
	ResourceLoader* resourceLoader,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
#if __APPLE__
	const std::string texturePathPrefix = "../../textures/";
#else
	const std::string texturePathPrefix = "../textures/";
#endif
	std::string texturePath = texturePathPrefix + mainTextureName;

	std::shared_ptr<TextureCreator> mainTexture =
		isRawTexture ?
		resourceLoader->GetRawTexture(mainTextureName) :
		resourceLoader->GetTexture(texturePath, gfxDeviceManager,
			logicalDeviceManager, commandPool);

	return std::make_shared<Material>(mainTexture,
		materialEnumType, materialNode);
}

std::shared_ptr<Material> GameObjectCreator::CreateMaterial(
	DescriptorSetFunctions::MaterialType materialEnumType,
	glm::vec4 const& tintColor) {
	return std::make_shared<Material>(materialEnumType, tintColor);
}

std::shared_ptr<Model> GameObjectCreator::LoadModelFromName(
	std::string const& modelName,
	ResourceLoader* resourceLoader) {
#if __APPLE__
	std::string modelPath = "../../models/";
#else
	std::string modelPath = "../models/";
#endif
	modelPath += modelName;
	return resourceLoader->GetModel(modelPath);
}

