
#include "BasicTurret.h"
#include "GameObjectCreationUtilFuncs.h"
#include "StationaryGameObjectBehavior.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

BasicTurret::BasicTurret(Scene* const scene,
	std::shared_ptr<GameObjectBehavior> behavior,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader,
	VkCommandPool commandPool) : GameObject(behavior, nullptr, nullptr) {
	// base of turret
	auto boxCenter = glm::vec3(0.5f, 0.5f, 0.5f);
	auto rightVec = glm::vec3(1.0f, 0.0f, 0.0f);
	auto upVec =  glm::vec3(0.0f, 1.0f, 0.0f);
	auto forwardVec = glm::vec3(0.0f, 0.0f, 1.0f);
	auto baseModel = Model::CreateBox(boxCenter, rightVec,upVec, forwardVec);
	nlohmann::json metadataNode = {
		{"tint_color",{1.0f, 0.0f, 0.0f, 1.0f }}
	};

	auto material = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto baseBehavior = std::make_shared<StationaryGameObjectBehavior>(scene);
	glm::mat4 relativeTransform(1.0f);
	relativeTransform = glm::translate(relativeTransform,
		glm::vec3(0.0f,-1.0f, 0.0f));
	auto constructedGameObject = GameObjectCreator::CreateMeshGameObject(
		material, baseModel, baseBehavior,
		relativeTransform, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
}