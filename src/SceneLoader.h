#pragma once

#include <string>

class SceneLoader
{
public:
	static void DeserializeJSONFileIntoScen(
		class Scene* scene,
		const std::string& jsonFilePath
	);

private:

};
