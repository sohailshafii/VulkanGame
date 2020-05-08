#include "SceneManagement/SceneLoader.h"

#include "GameObjects/GameObject.h"
#include "GameObjects/AIGameObjectBehavior.h"
#include "GameObjects/PlayerGameObjectBehavior.h"
#include "GameObjects/StationaryGameObjectBehavior.h"
#include "Resources/ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Material.h"
#include "Resources/ImageTextureLoader.h"
#include "Scene.h"
#include "nlohmann/json.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>

static void SetUpGameObject(const nlohmann::json& jsonObj,
	std::shared_ptr<GameObject>& constructedGameObject,
	ResourceLoader* resourceLoader,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool);

static void SetupMaterial(const nlohmann::json& materialNode,
						  std::shared_ptr<Material>& material,
						  ResourceLoader* resourceLoader,
						  GfxDeviceManager *gfxDeviceManager,
						  std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
						  VkCommandPool commandPool);

void SceneLoader::DeserializeJSONFileIntoScene(
	ResourceLoader* resourceLoader,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool,
	Scene* scene,
	const std::string& jsonFilePath) {
	try {
		std::ifstream jsonFile(jsonFilePath);
		nlohmann::json jsonObject;

		jsonFile >> jsonObject;

		nlohmann::json gameObjects = jsonObject["game_objects"];
		for (auto& element : gameObjects.items()) {
			std::shared_ptr<GameObject> constructedGameObject;
			SetUpGameObject(element.value(), constructedGameObject,
							resourceLoader, gfxDeviceManager,
							logicalDeviceManager, commandPool);
			scene->AddGameObject(constructedGameObject);
		}
	}
	catch (const std::exception& e) {
		std::stringstream exceptionMsg;
		exceptionMsg << "Could not deserialize JSON file: " << jsonFilePath
			<< ". Reason: " << e.what() << std::endl;
		throw exceptionMsg;
	}
}

static bool ContainsToken(const nlohmann::json& jsonObj,
						  const std::string& key) {
	return (jsonObj.find(key) != jsonObj.end());
}

static inline nlohmann::json SafeGetToken(const nlohmann::json& jsonObj,
	const std::string& key) {
	if (ContainsToken(jsonObj, key)) {
		return jsonObj[key];
	}
	std::stringstream exceptionMsg;
	exceptionMsg << "Could not find key: " << key
		<< " in JSON object: " << jsonObj << ".\n";
	throw exceptionMsg;
}

static void SetUpGameObject(const nlohmann::json& jsonObj,
	std::shared_ptr<GameObject>& constructedGameObject,
	ResourceLoader* resourceLoader,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
	std::string modelType = SafeGetToken(jsonObj, "model");
	std::string objectType = SafeGetToken(jsonObj, "type");
	auto objectPosition = SafeGetToken(jsonObj, "position");
	auto materialNode = SafeGetToken(jsonObj, "material");

	std::shared_ptr<GameObjectBehavior> GameObjectBehavior;
	if (objectType == "Player") {
		GameObjectBehavior = std::make_shared<PlayerGameObjectBehavior>();
	}
	else if (objectType == "AI") {
		GameObjectBehavior = std::make_shared<AIGameObjectBehavior>();
	}
	else {
		GameObjectBehavior = std::make_shared<StationaryGameObjectBehavior>();
	}
	
	std::shared_ptr<Material> newMaterial;
	SetupMaterial(materialNode, newMaterial, resourceLoader,
				  gfxDeviceManager, logicalDeviceManager,
				  commandPool);
}

static void SetupMaterial(const nlohmann::json& materialNode,
						  std::shared_ptr<Material>& material,
						  ResourceLoader* resourceLoader,
						  GfxDeviceManager *gfxDeviceManager,
						  std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager, VkCommandPool commandPool) {
	std::string materialToken = SafeGetToken(materialNode, "type");
	if (materialToken == "None")
	{
		// break out early as this doesn't require rendering
		return;
	}
	
	std::string mainTextureName = SafeGetToken(materialNode, "main_texture");
	
	#if __APPLE__
		const std::string texturePathPrefix = "../../textures/";
	#else
		const std::string texturePathPrefix = "../textures/";
	#endif
	std::string texturePath = texturePathPrefix + mainTextureName;
	
	std::shared_ptr<ImageTextureLoader> mainTexture = resourceLoader->GetTexture(texturePath, gfxDeviceManager, logicalDeviceManager, commandPool);
	DescriptorSetFunctions::MaterialType materialEnumType =
	DescriptorSetFunctions::MaterialType::UnlitTintedTextured;
	if (materialToken == "SimpleLambertian")
	{
		materialEnumType = DescriptorSetFunctions::MaterialType::SimpleLambertian;
	}
	
	material = std::make_shared<Material>(mainTexture,
										  materialEnumType);
}

