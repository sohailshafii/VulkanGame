#include "SceneManagement/SceneLoader.h"

#include "GameObjects/GameObject.h"
#include "GameObjects/GameObjectBehavior.h"
#include "GameObjects/MothershipBehavior.h"
#include "GameObjects/PlayerGameObjectBehavior.h"
#include "GameObjects/StationaryGameObjectBehavior.h"
#include "Resources/ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Material.h"
#include "Resources/ImageTextureLoader.h"
#include "Resources/Model.h"
#include "Scene.h"
#include "nlohmann/json.hpp"
#include "Math/PerlinNoise.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>

static void AdjustSceneSettings(const nlohmann::json& jsonObj,
	SceneLoader::SceneSettings& sceneSettings);

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

static void SetupTransformation(const nlohmann::json& transformNode,
								glm::mat4& localToWorld);

static std::unique_ptr<GameObjectBehavior> SetupGameObjectBehavior(const nlohmann::json& gameObjectNode);

void SceneLoader::DeserializeJSONFileIntoScene(
	ResourceLoader* resourceLoader,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool,
	Scene * const scene,
	SceneSettings& sceneSettings,
	const std::string& jsonFilePath) {
	try {
		std::ifstream jsonFile(jsonFilePath);
		nlohmann::json jsonObject;

		jsonFile >> jsonObject;

		nlohmann::json sceneSettingsNode = jsonObject["scene_settings"];
		AdjustSceneSettings(sceneSettingsNode, sceneSettings);

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

void AdjustSceneSettings(const nlohmann::json& jsonObj,
	SceneLoader::SceneSettings& sceneSettings) {
	auto cameraNode = jsonObj["camera"];

	auto cameraPosition = cameraNode["position"];
	float cameraYaw = cameraNode["yaw"];
	float cameraPitch = cameraNode["pitch"];
	float cameraMovementSpeed = cameraNode["movement_speed"];
	float cameraMouseSensitivity = cameraNode["mouse_sensitivity"];

	sceneSettings.cameraPosition = glm::vec3((float)cameraPosition[0], (float)cameraPosition[1],
		(float)cameraPosition[2]);
	sceneSettings.cameraYaw = cameraYaw;
	sceneSettings.cameraPitch = cameraPitch;
	sceneSettings.cameraMovementSpeed = cameraMovementSpeed;
	sceneSettings.cameraMouseSensitivity = cameraMouseSensitivity;
}

static void SetUpGameObject(const nlohmann::json& jsonObj,
	std::shared_ptr<GameObject>& constructedGameObject,
	ResourceLoader* resourceLoader,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
	std::string modelType = SafeGetToken(jsonObj, "model");
	std::string objectType = SafeGetToken(jsonObj, "type");
	auto materialNode = SafeGetToken(jsonObj, "material");

	std::shared_ptr<GameObjectBehavior> GameObjectBehavior;
	if (objectType == "Player") {
		GameObjectBehavior = std::make_shared<PlayerGameObjectBehavior>();
	}
	else if (objectType == "AI") {
		GameObjectBehavior = std::make_shared<MothershipBehavior>();
	}
	else {
		GameObjectBehavior = std::make_shared<StationaryGameObjectBehavior>();
	}
	
	std::shared_ptr<Material> newMaterial;
	SetupMaterial(materialNode, newMaterial, resourceLoader,
				  gfxDeviceManager, logicalDeviceManager,
				  commandPool);

	#if __APPLE__
		const std::string modelPathPrefix = "../../models/";
	#else
		const std::string modelPathPrefix = "../models/";
	#endif
	std::string modelPath = modelPathPrefix;
	std::shared_ptr<Model> gameObjectModel;
	// TODO: allow having no model
	if (modelType.find("Procedural") != std::string::npos)
	{
		auto metaDataNode = SafeGetToken(jsonObj, "meta_data");
		std::string primitiveType = SafeGetToken(metaDataNode, "primitive_type");
		
		if (primitiveType == "quad") {
			auto lowerLeftPos = SafeGetToken(metaDataNode, "lower_left");
			auto side1Vec = SafeGetToken(metaDataNode, "side_1_vec");
			auto side2Vec = SafeGetToken(metaDataNode, "side_2_vec");
			unsigned int numSide1Pnts = SafeGetToken(metaDataNode, "num_side_1_points");
			unsigned int numSide2Pnts = SafeGetToken(metaDataNode, "num_side_2_points");
			
			gameObjectModel = Model::CreatePlane(
			glm::vec3((float)lowerLeftPos[0], (float)lowerLeftPos[1], (float)lowerLeftPos[2]),
			glm::vec3((float)side1Vec[0], (float)side1Vec[1], (float)side1Vec[2]),
			glm::vec3((float)side2Vec[0], (float)side2Vec[1], (float)side2Vec[2]),
			numSide1Pnts, numSide2Pnts,
			NoiseGeneratorType::None);
		}
		else if (primitiveType == "noisy_quad") {
			auto metaDataNode = SafeGetToken(jsonObj, "meta_data");
			auto lowerLeftPos = SafeGetToken(metaDataNode, "lower_left");
			auto side1Vec = SafeGetToken(metaDataNode, "side_1_vec");
			auto side2Vec = SafeGetToken(metaDataNode, "side_2_vec");
			unsigned int numSide1Pnts = SafeGetToken(metaDataNode, "num_side_1_points");
			unsigned int numSide2Pnts = SafeGetToken(metaDataNode, "num_side_2_points");
			
			std::string noiseType = SafeGetToken(metaDataNode, "noise_type");
			if (noiseType == "perlin") {
				uint32_t numNoiseLayers = SafeGetToken(metaDataNode, "num_noise_layers");
				gameObjectModel = Model::CreatePlane(
				glm::vec3((float)lowerLeftPos[0], (float)lowerLeftPos[1], (float)lowerLeftPos[2]),
				glm::vec3((float)side1Vec[0], (float)side1Vec[1], (float)side1Vec[2]),
				glm::vec3((float)side2Vec[0], (float)side2Vec[1], (float)side2Vec[2]),
				numSide1Pnts, numSide2Pnts,
				NoiseGeneratorType::Perlin, numNoiseLayers);
			}
			else {
				std::stringstream exceptionMsg;
				exceptionMsg << "Don't understand noise type: " << noiseType;
				throw exceptionMsg;
			}
		}
		else if (primitiveType == "icosahedron") {
			float radius = SafeGetToken(metaDataNode, "radius");
			uint32_t numSubdiv = SafeGetToken(metaDataNode, "num_subdivisions");
			gameObjectModel = Model::CreateIcosahedron(radius, numSubdiv);
		}
		else {
			std::stringstream exceptionMsg;
			exceptionMsg << "Don't understand primitive type: " << primitiveType;
			throw exceptionMsg;
		}
	}
	else if (modelType != "None") {
		modelPath += modelType;
		gameObjectModel = resourceLoader->GetModel(modelPath);
	}
	
	constructedGameObject  =
		std::make_shared<GameObject>(gameObjectModel, newMaterial,
									 SetupGameObjectBehavior(jsonObj), gfxDeviceManager,
									 logicalDeviceManager, commandPool);
	
	auto transformationNode = SafeGetToken(jsonObj, "transformation");
	glm::mat4 localToWorldTranfsorm(1.0f);
	SetupTransformation(transformationNode, localToWorldTranfsorm);
	constructedGameObject->SetModelTransform(localToWorldTranfsorm);
}

static void SetupMaterial(const nlohmann::json& materialNode,
						  std::shared_ptr<Material>& material,
						  ResourceLoader* resourceLoader,
						  GfxDeviceManager *gfxDeviceManager,
						  std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager, VkCommandPool commandPool) {
	std::string materialToken = SafeGetToken(materialNode, "type");
	// TODO: make material optional
	if (materialToken == "None") {
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
	if (materialToken == "WavySurface") {
		materialEnumType = DescriptorSetFunctions::MaterialType::WavySurface;
	}
	else if (materialToken == "BumpySurface") {
		materialEnumType = DescriptorSetFunctions::MaterialType::BumpySurface;
	}
	
	material = std::make_shared<Material>(mainTexture,
										  materialEnumType);
}

static void SetupTransformation(const nlohmann::json& transformNode,
								glm::mat4& localToWorld) {
	localToWorld = glm::mat4(1.0f);
	if (ContainsToken(transformNode, "position")) {
		auto positionNode = SafeGetToken(transformNode, "position");
		localToWorld = glm::translate(localToWorld,
									  glm::vec3((float)positionNode[0],
												(float)positionNode[1],
												(float)positionNode[2]));
	}
	if (ContainsToken(transformNode, "rotation")) {
		auto rotationNode = SafeGetToken(transformNode, "rotation");
		localToWorld = glm::rotate(localToWorld,
									  glm::radians((float)rotationNode[0]),
									  glm::vec3(1.0f, 0.0f, 0.0f));
		localToWorld = glm::rotate(localToWorld,
									  glm::radians((float)rotationNode[1]),
									  glm::vec3(0.0f, 1.0f, 0.0f));
		localToWorld = glm::rotate(localToWorld,
									  glm::radians((float)rotationNode[2]),
									  glm::vec3(0.0f, 0.0f, 1.0f));
		
	}
	if (ContainsToken(transformNode, "scale")) {
		auto scaleNode = SafeGetToken(transformNode, "scale");
		localToWorld = glm::scale(localToWorld,
								  glm::vec3((float)scaleNode[0],
											(float)scaleNode[1],
											(float)scaleNode[2]));
	}
}

std::unique_ptr<GameObjectBehavior> SetupGameObjectBehavior(const nlohmann::json& gameObjectNode) {
	std::string gameObjectBehaviorStr = gameObjectNode["type"];
	std::unique_ptr<GameObjectBehavior> newGameObjBehavior;
	if (gameObjectBehaviorStr == "Stationary") {
		newGameObjBehavior = std::make_unique<StationaryGameObjectBehavior>();
	}
	else if (gameObjectBehaviorStr == "Player") {
		newGameObjBehavior = std::make_unique<PlayerGameObjectBehavior>();
	}
	else if (gameObjectBehaviorStr == "Mothership") {
		newGameObjBehavior = std::make_unique<MothershipBehavior>();
	}
	else {
		std::stringstream exceptionMsg;
		exceptionMsg << "Don't understand game object behavior type: " << gameObjectBehaviorStr;
		throw exceptionMsg;
	}
	
	return newGameObjBehavior;
}
