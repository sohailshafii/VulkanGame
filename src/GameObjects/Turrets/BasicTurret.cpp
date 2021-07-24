
#include "BasicTurret.h"
#include "GameObjectCreationUtilFuncs.h"
#include "StationaryGameObjectBehavior.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/MeshGameObject.h"
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
	childGameObjects.push_back(std::static_pointer_cast<GameObject>
		(constructedGameObject));

	// top of turret
	boxCenter = glm::vec3(0.5f, 0.75f, 0.5f);
	rightVec = glm::vec3(25.0f, 0.0f, 0.0f);
	upVec = glm::vec3(0.0f, 25.0f, 0.0f);
	forwardVec = glm::vec3(0.0f, 0.0f, 25.0f);
	auto turretTopModel = Model::CreateBox(boxCenter, rightVec, upVec, forwardVec);
	metadataNode = {
		{"tint_color",{0.0f, 1.0f, 0.0f, 1.0f }}
	};
	auto topMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto topBehavior = std::make_shared<StationaryGameObjectBehavior>(scene);
	relativeTransform = glm::mat4(1.0f);
	auto turretTop = GameObjectCreator::CreateMeshGameObject(
		topMaterial, turretTopModel, topBehavior,
		relativeTransform, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	childGameObjects.push_back(std::static_pointer_cast<GameObject>
		(turretTop));
}