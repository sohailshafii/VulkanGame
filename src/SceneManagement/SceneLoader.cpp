#include "SceneManagement/SceneLoader.h"

#include "GameObjects/GameObject.h"
#include "GameObjects/GameObjectBehavior.h"
#include "GameObjects/Mothership/MothershipBehavior.h"
#include "GameObjects/Mothership/PawnBehavior.h"
#include "GameObjects/Player/PlayerGameObjectBehavior.h"
#include "GameObjects/Msc/StationaryGameObjectBehavior.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "Resources/ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Material.h"
#include "Resources/TextureCreator.h"
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
	Scene* const scene,
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

static std::shared_ptr<GameObjectBehavior> SetupGameObjectBehavior(
	const nlohmann::json& gameObjectNode,
	Scene* const scene);

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
							scene, resourceLoader, gfxDeviceManager,
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
	Scene* const scene,
	ResourceLoader* resourceLoader,
	GfxDeviceManager *gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
	std::string modelType = SafeGetToken(jsonObj, "model");
	auto materialNode = SafeGetToken(jsonObj, "material");
	
	std::shared_ptr<Material> newMaterial;
	SetupMaterial(materialNode, newMaterial, resourceLoader,
				  gfxDeviceManager, logicalDeviceManager,
				  commandPool);

	std::shared_ptr<Model> gameObjectModel;
	if (modelType.find("Procedural") != std::string::npos)
	{
		auto metaDataNode = SafeGetToken(jsonObj, "meta_data");
		std::string primitiveType = SafeGetToken(metaDataNode, "primitive_type");
		
		if (primitiveType == "quad") {
			auto lowerLeftPos = SafeGetToken(metaDataNode, "lower_left");
			auto side1Vec = SafeGetToken(metaDataNode, "side_1_vec");
			auto side2Vec = SafeGetToken(metaDataNode, "side_2_vec");
			
			gameObjectModel = Model::CreateQuad(
				glm::vec3((float)lowerLeftPos[0], (float)lowerLeftPos[1], (float)lowerLeftPos[2]),
				glm::vec3((float)side1Vec[0], (float)side1Vec[1], (float)side1Vec[2]),
				glm::vec3((float)side2Vec[0], (float)side2Vec[1], (float)side2Vec[2]),
				true);
		}
		else if (primitiveType == "plane") {
			auto metaDataNode = SafeGetToken(jsonObj, "meta_data");
			auto lowerLeftPos = SafeGetToken(metaDataNode, "lower_left");
			auto side1Vec = SafeGetToken(metaDataNode, "side_1_vec");
			auto side2Vec = SafeGetToken(metaDataNode, "side_2_vec");
			unsigned int numSide1Pnts = SafeGetToken(metaDataNode, "num_side_1_points");
			unsigned int numSide2Pnts = SafeGetToken(metaDataNode, "num_side_2_points");
			
			std::string noiseType = SafeGetToken(metaDataNode, "noise_type");
			if (noiseType == "perlin" || noiseType == "none") {
				uint32_t numNoiseLayers = ContainsToken(metaDataNode, "num_noise_layers") ? 
					SafeGetToken(metaDataNode, "num_noise_layers") : 0;
				gameObjectModel = Model::CreatePlane(
					glm::vec3((float)lowerLeftPos[0], (float)lowerLeftPos[1], (float)lowerLeftPos[2]),
					glm::vec3((float)side1Vec[0], (float)side1Vec[1], (float)side1Vec[2]),
					glm::vec3((float)side2Vec[0], (float)side2Vec[1], (float)side2Vec[2]),
					numSide1Pnts, numSide2Pnts,
					noiseType == "perlin" ? NoiseGeneratorType::Perlin : NoiseGeneratorType::None,
					numNoiseLayers);
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
		gameObjectModel = GameObjectCreator::LoadModelFromName(
			modelType, resourceLoader);
	}
	
	auto transformationNode = SafeGetToken(jsonObj, "transformation");
	glm::mat4 localToWorldTransform(1.0f);
	SetupTransformation(transformationNode, localToWorldTransform);
	std::shared_ptr<GameObjectBehavior> gameObjectBehavior =
		SetupGameObjectBehavior(jsonObj, scene);
	constructedGameObject = GameObjectCreator::CreateGameObject(
		newMaterial, gameObjectModel, gameObjectBehavior,
		localToWorldTransform, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
}

static void SetupMaterial(const nlohmann::json& materialNode,
						  std::shared_ptr<Material>& material,
						  ResourceLoader* resourceLoader,
						  GfxDeviceManager *gfxDeviceManager,
						  std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
						VkCommandPool commandPool) {
	std::string materialToken = SafeGetToken(materialNode, "type");
	std::string mainTextureName = SafeGetToken(materialNode, "main_texture");
	
	DescriptorSetFunctions::MaterialType materialEnumType =
	DescriptorSetFunctions::MaterialType::UnlitTintedTextured;
	if (materialToken == "UnlitColor") {
		materialEnumType = DescriptorSetFunctions::MaterialType::UnlitColor;
	}
	else if (materialToken == "Text") {
		materialEnumType = DescriptorSetFunctions::MaterialType::Text;
	}
	else if (materialToken == "WavySurface") {
		materialEnumType = DescriptorSetFunctions::MaterialType::WavySurface;
	}
	else if (materialToken == "BumpySurface") {
		materialEnumType = DescriptorSetFunctions::MaterialType::BumpySurface;
	}
	else if (materialToken == "MotherShip") {
		materialEnumType = DescriptorSetFunctions::MaterialType::MotherShip;
	}
	
	material = GameObjectCreator::CreateMaterial(materialEnumType,
		mainTextureName, false, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
}

static void SetupTransformation(const nlohmann::json& transformNode,
								glm::mat4& localToWorld) {
	localToWorld = glm::mat4(1.0f);
	if (ContainsToken(transformNode, "position")) {
		auto positionNode = SafeGetToken(transformNode, "position");
		auto position = glm::vec3((float)positionNode[0],
					(float)positionNode[1],
					(float)positionNode[2]);
		localToWorld = glm::translate(localToWorld,
			position);
	}
	if (ContainsToken(transformNode, "rotation")) {
		auto rotationNode = SafeGetToken(transformNode, "rotation");
		auto rotation = glm::vec3((float)rotationNode[0],
			(float)rotationNode[1], (float)rotationNode[2]);
		localToWorld = glm::rotate(localToWorld,
									glm::radians(rotation[0]),
									glm::vec3(1.0f, 0.0f, 0.0f));
		localToWorld = glm::rotate(localToWorld,
									glm::radians(rotation[1]),
									glm::vec3(0.0f, 1.0f, 0.0f));
		localToWorld = glm::rotate(localToWorld,
									glm::radians(rotation[2]),
									glm::vec3(0.0f, 0.0f, 1.0f));
		
	}
	if (ContainsToken(transformNode, "scale")) {
		auto scaleNode = SafeGetToken(transformNode, "scale");
		auto scale = glm::vec3((float)scaleNode[0], (float)scaleNode[1],
			(float)scaleNode[2]);
		localToWorld = glm::scale(localToWorld,
									glm::vec3(scale[0], scale[1],
									scale[2]));
	}
}

std::shared_ptr<GameObjectBehavior> SetupGameObjectBehavior(
	const nlohmann::json& gameObjectNode,
	Scene* const scene) {
	std::string gameObjectBehaviorStr = gameObjectNode["type"];
	std::shared_ptr<GameObjectBehavior> newGameObjBehavior;
	if (gameObjectBehaviorStr == "Stationary") {
		newGameObjBehavior = std::make_shared<StationaryGameObjectBehavior>();
	}
	else if (gameObjectBehaviorStr == "Mothership") {
		float shipRadius = gameObjectNode["ship_radius"];
		newGameObjBehavior = std::make_shared<MothershipBehavior>(scene,
			shipRadius);
	}
	else if (gameObjectBehaviorStr == "Pawn") {
		newGameObjBehavior = std::make_shared<PawnBehavior>();
	}
	else {
		std::stringstream exceptionMsg;
		exceptionMsg << "Don't understand game object behavior type: " << gameObjectBehaviorStr;
		throw exceptionMsg;
	}
	
	return newGameObjBehavior;
}
