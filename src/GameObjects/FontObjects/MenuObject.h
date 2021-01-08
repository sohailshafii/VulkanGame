#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;
class Model;
class Material;
class GameObject;
class FontTextureBuffer;

/// <summary>
/// A menu item that renders as a phrase or sentence.
/// </summary>
class MenuObject {
public:
	MenuObject(std::string const & menuText,
		glm::vec3 const & objectPosition,
		glm::vec3 const & scale,
		bool isCentered,
		FontTextureBuffer* fontTextureBuffer,
		std::shared_ptr<Material> & gameObjectMaterial,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);

	std::vector<std::shared_ptr<GameObject>> const &
		GetTextGameObjects() {
		return textGameObjects;
	}

private:
	std::shared_ptr<Model> CreateModelForCharacter(
		unsigned char character,
		FontTextureBuffer* fontTextureBuffer,
		float& advanceVal,
		float scale);

	void ComputeWorldBoundsOfMenuObject(glm::vec3& min, glm::vec3& max,
		glm::vec3 const& worldScale);

	std::shared_ptr<Material> gameObjectMaterial;
	std::vector<std::shared_ptr<GameObject>> textGameObjects;
};
