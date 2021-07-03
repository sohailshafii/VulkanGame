#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include "vulkan/vulkan.h"
#include "DescriptorSetFunctions.h"
#include "NoiseGenerator.h"
#include "DescriptorSetFunctions.h"
#include "nlohmann/json.hpp"

class MeshGameObject;
class GameObjectBehavior;
class Material;
class Model;
class ResourceLoader;
class GfxDeviceManager;
class LogicalDeviceManager;

class GameObjectCreator {
public:
	static std::shared_ptr<MeshGameObject> CreateMeshGameObject(
		std::shared_ptr<Material> const& material,
		std::shared_ptr<Model> const& gameObjectModel,
		std::shared_ptr<GameObjectBehavior> gameObjectBehavior,
		glm::mat4 const& localToWorldTransform,
		ResourceLoader* resourceLoader,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);

	// create some useful functions for more complicated stuff
	static std::shared_ptr<Material> CreateMaterial(
		DescriptorSetFunctions::MaterialType materialEnumType,
		std::string const& mainTextureName,
		nlohmann::json const& materialNode,
		bool isRawTexture,
		ResourceLoader* resourceLoader,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		VkCommandPool commandPool);

	static std::shared_ptr<Material> CreateMaterial(
		DescriptorSetFunctions::MaterialType materialEnumType,
		nlohmann::json const& materialNode = nlohmann::json());

	static std::shared_ptr<Model> LoadModelFromName(
		std::string const& modelName,
		ResourceLoader* resourceLoader);
};
