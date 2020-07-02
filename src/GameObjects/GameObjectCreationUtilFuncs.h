#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include "vulkan/vulkan.h"
#include "DescriptorSetFunctions.h"
#include "NoiseGenerator.h"

class GameObject;
class GameObjectBehavior;
class Material;
class Model;
class ResourceLoader;
class GfxDeviceManager;
class LogicalDeviceManager;

class GameObjectCreator {
public:
	std::shared_ptr<GameObject> CreateUpGameObject(
		std::shared_ptr<Material> const& material,
		std::shared_ptr<Model> const& gameObjectModel,
		std::unique_ptr<GameObjectBehavior> gameObjectBehavior,
		glm::mat4 const& localToWorldTranfsorm,
		ResourceLoader* resourceLoader,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);

	// create some useful functions for more complicated stuff
	std::shared_ptr<Material> CreateMaterial(
		DescriptorSetFunctions::MaterialType materialEnumType,
		std::string const& mainTextureName,
		ResourceLoader* resourceLoader,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);

	std::shared_ptr<Model> CreatePlane(glm::vec3 const& lowerLeft,
		glm::vec3 const& side1Vec, glm::vec3 const& side2Vec,
		unsigned int numSide1Pnts, unsigned int numSide2Pnts,
		NoiseGeneratorType noiseGeneratorType);

	std::shared_ptr<Model> LoadModelFromPath(
		std::string const& modelName,
		ResourceLoader* resourceLoader);
};
