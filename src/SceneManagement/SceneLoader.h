#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "vulkan/vulkan.h"

class ResourceLoader;
class GfxDeviceManager;
class LogicalDeviceManager;

class SceneLoader {
public:
	struct SceneSettings {
		glm::vec3 cameraPosition;
		float cameraYaw;
		float cameraPitch;
		float cameraMovementSpeed;
		float cameraMouseSensitivity;
	};

	static void DeserializeJSONFileIntoScene(
		ResourceLoader* resourceLoader,
		GfxDeviceManager *gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager>const& logicalDeviceManager,
		VkCommandPool commandPool,
		class Scene * const scene,
		SceneSettings& sceneSettings,
		const std::string& jsonFilePath);

private:

};
