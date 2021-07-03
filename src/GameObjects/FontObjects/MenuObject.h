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
class MeshGameObject;
class FontTextureBuffer;
class FontGameObjectBehavior;

/// <summary>
/// A menu item that renders as a phrase or sentence.
/// </summary>
class MenuObject {
public:
	enum class MenuType : char { Play = 0, About, Difficulty,
		Quit, Easy, Medium, Hard, Back };

	// it's a vector because it can be multiple lines
	MenuObject(MenuType menuType, std::string const& menuText,
		glm::vec3 const & objectPosition,
		glm::vec3 const & scale,
		bool isCentered,
		FontTextureBuffer* fontTextureBuffer,
		std::string const& textureSheetName,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);

	std::shared_ptr<MeshGameObject> GetTextGameObject() {
		return textGameObject;
	}

	void SetSelectState(bool selectState);

	MenuType GetMenuType() const {
		return menuType;
	}

	std::string GetText() const {
		return menuText;
	}

	glm::vec3 GetScale() const {
		return computedScale;
	}

private:
	std::shared_ptr<Model> CreateModelForCharacter(
		unsigned char character,
		FontTextureBuffer* fontTextureBuffer,
		float& advanceValX,
		float advanceValY,
		float scale);

	void ComputeWorldBoundsOfMenuObject(glm::vec3& min, glm::vec3& max,
		glm::vec3 const& worldScale);

	std::shared_ptr<MeshGameObject> textGameObject;
	std::shared_ptr<FontGameObjectBehavior> behaviorObject;
	bool selectState;
	MenuType menuType;
	std::string menuText;
	glm::vec3 computedScale;
};
