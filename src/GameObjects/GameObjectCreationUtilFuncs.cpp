
#include "GameObjectCreationUtilFuncs.h"
#include "Resources/ResourceLoader.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/GameObjectBehavior.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Model.h"

std::shared_ptr<GameObject> GameObjectCreator::CreateUpGameObject(
	std::shared_ptr<Material> const & material,
	std::shared_ptr<Model> const & gameObjectModel,
	std::unique_ptr<GameObjectBehavior> gameObjectBehavior,
	glm::mat4 const & localToWorldTranfsorm,
	ResourceLoader* resourceLoader,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
	std::shared_ptr<GameObject> constructedGameObject =
		std::make_shared<GameObject>(gameObjectModel, material,
			// TODO: verify usage of unique_ptr here!
			std::move(gameObjectBehavior),
			gfxDeviceManager,
			logicalDeviceManager, commandPool);
	constructedGameObject->SetModelTransform(localToWorldTranfsorm);

	return constructedGameObject;
}

std::shared_ptr<Material> GameObjectCreator::CreateMaterial(
	DescriptorSetFunctions::MaterialType materialEnumType,
	std::string const & mainTextureName,
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

	std::shared_ptr<ImageTextureLoader> mainTexture =
		resourceLoader->GetTexture(texturePath, gfxDeviceManager,
			logicalDeviceManager, commandPool);

	return std::make_shared<Material>(mainTexture,
		materialEnumType);
}

std::shared_ptr<Model> GameObjectCreator::CreatePlane(glm::vec3 const& lowerLeft,
	glm::vec3 const& side1Vec, glm::vec3 const& side2Vec,
	unsigned int numSide1Pnts, unsigned int numSide2Pnts,
	NoiseGeneratorType noiseGeneratorType) {
	return Model::CreatePlane(lowerLeft, side1Vec, side2Vec,
		numSide1Pnts, numSide2Pnts, noiseGeneratorType);
}

std::shared_ptr<Model> GameObjectCreator::LoadModelFromPath(
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

