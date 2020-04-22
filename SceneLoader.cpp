#include "SceneLoader.h"
#include "GameObject.h"
#include "ThirdParty/nlohmann/json.hpp"
#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>

static void SetUpGameObject(const nlohmann::json& jsonObj,
	std::shared_ptr<GameObject>& constructedGameObject);

void SceneLoader::DeserializeJSONFileIntoScen(
	class Scene* scene,
	const std::string& jsonFilePath) {
	try {
		std::ifstream jsonFile(jsonFilePath);
		nlohmann::json jsonObject;

		jsonFile >> jsonObject;

		nlohmann::json gameObjects = jsonObject["game_objects"];
		for (auto& element : gameObjects.items()) {
			std::shared_ptr<GameObject> constructedGameObject;
			SetUpGameObject(element, constructedGameObject);
		}
	}
	catch (const std::exception& e) {
		std::stringstream exceptionMsg;
		exceptionMsg << "Could not deserialize JSON file: " << jsonFilePath
			<< ". Reason: " << e.what() << std::endl;
		throw exceptionMsg;
	}
}

static inline nlohmann::json SafeGetToken(const nlohmann::json& jsonObj,
	const std::string& key) {
	if (jsonObj.find(key) != jsonObj.end()) {
		return jsonObj[key];
	}
	std::stringstream exceptionMsg;
	exceptionMsg << "Could not find key: " << key
		<< " in JSON object: " << jsonObj << ".\n";
	throw exceptionMsg;
}

static void SetUpGameObject(const nlohmann::json& jsonObj,
	std::shared_ptr<GameObject>& constructedGameObject) {
	//std::string modelType = SafeGetToken(jsonObj, "model");
	//std::string objectType = SafeGetToken(jsonObj, "type");
	//auto objectPosition = SafeGetToken(jsonObj, "position");
	//std::string materialType = SafeGetToken(jsonObj, "material");
	// TODO: finish
}



