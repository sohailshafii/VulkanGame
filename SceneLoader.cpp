#include "SceneLoader.h"
#include "ThirdParty/nlohmann/json.hpp"
#include <exception>
#include <iostream>
#include <fstream>

void SceneLoader::DeserializeJSONFileIntoScen(
	class Scene* scene,
	const std::string& jsonFilePath) {
	try {
		std::ifstream jsonFile(jsonFilePath);
		nlohmann::json jsonObject;

		jsonFile >> jsonObject;


	}
	catch (const std::exception& e) {

	}
}

