#include "SceneManagement/SceneLoader.h"

#include "GameObjects/GameObject.h"
#include "GameObjects/MeshGameObject.h"
#include "GameObjects/GameObjectBehavior.h"
#include "GameObjects/Mothership/Mothership.h"
#include "GameObjects/Mothership/MothershipBehavior.h"
#include "GameObjects/Mothership/PawnBehavior.h"
#include "GameObjects/GameObjectBehavior.h"
#include "GameObjects/Player/PlayerGameObjectBehavior.h"
#include "GameObjects/Msc/StationaryGameObjectBehavior.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Turrets/BasicTurretBehavior.h"
#include "GameObjects/Turrets/BasicTurret.h"
#include "Resources/ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "Resources/Material.h"
#include "Resources/TextureCreator.h"
#include "Resources/Model.h"
#include "Scene.h"
#include "nlohmann/json.hpp"
#include "Math/PerlinNoise.h"
#include "Common.h"
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
	std::string modelType = Common::SafeGetToken(jsonObj, "model");
	auto materialNode = Common::SafeGetToken(jsonObj, "material");
	
	std::shared_ptr<Material> newMaterial;
	SetupMaterial(materialNode, newMaterial, resourceLoader,
				  gfxDeviceManager, logicalDeviceManager,
				  commandPool);

	std::shared_ptr<Model> gameObjectModel;
	if (modelType.find("Procedural") != std::string::npos)
	{
		auto metaDataNode = Common::SafeGetToken(jsonObj, "meta_data");
		std::string primitiveType = Common::SafeGetToken(metaDataNode, "primitive_type");
		
		if (primitiveType == "quad") {
			auto lowerLeftPos = Common::SafeGetToken(metaDataNode, "lower_left");
			auto side1Vec = Common::SafeGetToken(metaDataNode, "side_1_vec");
			auto side2Vec = Common::SafeGetToken(metaDataNode, "side_2_vec");
			
			gameObjectModel = Model::CreateQuad(
				glm::vec3((float)lowerLeftPos[0], (float)lowerLeftPos[1], (float)lowerLeftPos[2]),
				glm::vec3((float)side1Vec[0], (float)side1Vec[1], (float)side1Vec[2]),
				glm::vec3((float)side2Vec[0], (float)side2Vec[1], (float)side2Vec[2]),
				true);
		}
		else if (primitiveType == "box") {
			auto boxCenter = Common::SafeGetToken(metaDataNode, "box_center");
			auto rightVec = Common::SafeGetToken(metaDataNode, "right_vec");
			auto upVec = Common::SafeGetToken(metaDataNode, "up_vec");
			auto forwardVec = Common::SafeGetToken(metaDataNode, "forward_vec");

			gameObjectModel = Model::CreateBox(
				glm::vec3((float)boxCenter[0], (float)boxCenter[1], (float)boxCenter[2]),
				glm::vec3((float)rightVec[0], (float)rightVec[1], (float)rightVec[2]),
				glm::vec3((float)upVec[0], (float)upVec[1], (float)upVec[2]),
				glm::vec3((float)forwardVec[0], (float)forwardVec[1], (float)forwardVec[2]));
		}
		else if (primitiveType == "plane") {
			auto metaDataNode = Common::SafeGetToken(jsonObj, "meta_data");
			auto lowerLeftPos = Common::SafeGetToken(metaDataNode, "lower_left");
			auto side1Vec = Common::SafeGetToken(metaDataNode, "side_1_vec");
			auto side2Vec = Common::SafeGetToken(metaDataNode, "side_2_vec");
			unsigned int numSide1Pnts = Common::SafeGetToken(metaDataNode, "num_side_1_points");
			unsigned int numSide2Pnts = Common::SafeGetToken(metaDataNode, "num_side_2_points");
			
			std::string noiseType = Common::SafeGetToken(metaDataNode, "noise_type");
			if (noiseType == "perlin" || noiseType == "none") {
				uint32_t numNoiseLayers = Common::ContainsToken(metaDataNode, "num_noise_layers") ?
					Common::SafeGetToken(metaDataNode, "num_noise_layers") : 0;
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
			float radius = Common::SafeGetToken(metaDataNode, "radius");
			uint32_t numSubdiv = Common::SafeGetToken(metaDataNode, "num_subdivisions");
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
	
	auto transformationNode = Common::SafeGetToken(jsonObj, "transformation");
	glm::mat4 localToWorldTransform(1.0f);
	SetupTransformation(transformationNode, localToWorldTransform);

	std::string gameObjectType = jsonObj["type"];
	if (gameObjectType == "BasicTurret") {
		constructedGameObject = std::make_shared<BasicTurret>
			(scene, std::make_shared<BasicTurretBehavior>(), gfxDeviceManager,
			logicalDeviceManager, resourceLoader, commandPool, localToWorldTransform);
	}
	else if (gameObjectType == "Mothership") {
		constructedGameObject = std::make_shared<Mothership>(
			std::make_shared<MothershipBehavior>(scene, jsonObj["ship_radius"]),
			gfxDeviceManager, logicalDeviceManager, commandPool, gameObjectModel,
			newMaterial, localToWorldTransform);
	}
	else if (gameObjectType == "Stationary" || gameObjectType == "Pawn") {
		std::shared_ptr<GameObjectBehavior> gameObjectBehavior = nullptr;
		if (gameObjectType == "Pawn") {
			gameObjectBehavior = std::make_shared<PawnBehavior>();
		}
		else {
			gameObjectBehavior = std::make_shared<StationaryGameObjectBehavior>();
		}

		constructedGameObject = GameObjectCreator::CreateMeshGameObject(
			newMaterial, gameObjectModel, gameObjectBehavior,
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	}
	else {
		std::stringstream exceptionMsg;
			exceptionMsg << "Could not understand game object type: " << gameObjectType
			<< std::endl;
			throw exceptionMsg;
	}
}

static void SetupMaterial(const nlohmann::json& materialNode,
	std::shared_ptr<Material>& material,
	ResourceLoader* resourceLoader,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	VkCommandPool commandPool) {
	std::string materialToken = Common::SafeGetToken(materialNode, "type");
	std::string mainTextureName = Common::SafeGetToken(materialNode, "main_texture");

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
	nlohmann::json metadataNode;
	if (Common::ContainsToken(materialNode, "meta_data")) {
		metadataNode = Common::SafeGetToken(materialNode, "meta_data");
	}
	material = GameObjectCreator::CreateMaterial(materialEnumType,
		mainTextureName, metadataNode, false, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
}

static void SetupTransformation(const nlohmann::json& transformNode,
								glm::mat4& localToWorld) {
	localToWorld = glm::mat4(1.0f);
	if (Common::ContainsToken(transformNode, "position")) {
		auto positionNode = Common::SafeGetToken(transformNode, "position");
		auto position = glm::vec3((float)positionNode[0],
					(float)positionNode[1],
					(float)positionNode[2]);
		localToWorld = glm::translate(localToWorld,
			position);
	}
	if (Common::ContainsToken(transformNode, "rotation")) {
		auto rotationNode = Common::SafeGetToken(transformNode, "rotation");
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
	if (Common::ContainsToken(transformNode, "scale")) {
		auto scaleNode = Common::SafeGetToken(transformNode, "scale");
		auto scale = glm::vec3((float)scaleNode[0], (float)scaleNode[1],
			(float)scaleNode[2]);
		localToWorld = glm::scale(localToWorld,
									glm::vec3(scale[0], scale[1],
									scale[2]));
	}
}
