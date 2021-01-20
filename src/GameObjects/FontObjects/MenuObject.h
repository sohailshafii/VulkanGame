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
class FontGameObjectBehavior;

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
		std::string const& textureSheetName,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);

	std::vector<std::shared_ptr<GameObject>> const &
		GetTextGameObjects() {
		return textGameObjects;
	}

	void SetSelectState(bool selectState);

private:
	std::shared_ptr<Model> CreateModelForCharacter(
		unsigned char character,
		FontTextureBuffer* fontTextureBuffer,
		float& advanceVal,
		float scale);

	void ComputeWorldBoundsOfMenuObject(glm::vec3& min, glm::vec3& max,
		glm::vec3 const& worldScale);

	std::vector<std::shared_ptr<GameObject>> textGameObjects;
	std::vector<std::shared_ptr<FontGameObjectBehavior>> behaviorObjects;
	bool selectState;
};
