
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
	VkCommandPool commandPool,
	glm::mat4 const& localToWorldTransform) : GameObject(behavior) {
	localTransform = localToWorldTransform;
	localToWorld = localToWorldTransform;

	// base of turret
	auto boxCenter = glm::vec3(0.0f,-0.1f, 0.0f);
	auto rightVec = glm::vec3(1.0f, 0.0f, 0.0f);
	auto upVec = glm::vec3(0.0f, 0.2f, 0.0f);
	auto forwardVec = glm::vec3(0.0f, 0.0f, 1.0f);
	auto baseModel = Model::CreateBox(boxCenter, rightVec, upVec, forwardVec);
	nlohmann::json metadataNode = {
		{"tint_color",{0.0f, 1.0f, 0.0f, 1.0f }}
	};
	auto baseMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto baseBehavior = std::make_shared<StationaryGameObjectBehavior>(scene);
	glm::mat4 relativeTransform(1.0f);
	relativeTransform = glm::translate(relativeTransform,
		glm::vec3(0.0f, -0.2f, 0.0));
	AddSubMesh(baseMaterial, baseModel, baseBehavior,
		relativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool,
		"turretBase");

	// body of turret
	boxCenter = glm::vec3(0.0f, 0.5f, 0.0f);
	rightVec = glm::vec3(1.0f, 0.0f, 0.0f);
	upVec =  glm::vec3(0.0f, 1.0f, 0.0f);
	forwardVec = glm::vec3(0.0f, 0.0f, 1.0f);
	auto bodyModel = Model::CreateBox(boxCenter, rightVec, upVec, forwardVec);
	name = "BasicTurret";
	metadataNode = {
		{"tint_color",{1.0f, 0.0f, 0.0f, 1.0f }}
	};
	auto bodyMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	relativeTransform = glm::mat4(1.0f);
	relativeTransform = glm::translate(relativeTransform,
		glm::vec3(0.0f,-0.2f, 0.0));
	AddSubMesh(bodyMaterial, bodyModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		relativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool,
		"turretBody");

	// top of turret
	/*boxCenter = glm::vec3(0.5f, 1.25f, 0.5f);
	rightVec = glm::vec3(0.50f, 0.0f, 0.0f);
	upVec = glm::vec3(0.0f, 0.50f, 0.0f);
	forwardVec = glm::vec3(0.0f, 0.0f, 0.50f);*/
	auto turretTopModel = Model::CreateIcosahedron(0.25f, 2);
	metadataNode = {
		{"tint_color",{0.0f, 1.0f, 0.0f, 1.0f }}
	};
	auto topMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	relativeTransform = glm::mat4(1.0f);
	relativeTransform = glm::translate(relativeTransform,
		glm::vec3(0.0f, 1.25f, 0.0f));
	AddSubMesh(topMaterial, turretTopModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		relativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool,
		"turretTop");

	// TODO: build basic turret structure
	// TODO:  make turret AI
}

void BasicTurret::AddSubMesh(
	std::shared_ptr<Material> const & material,
	std::shared_ptr<Model> const & baseModel,
	std::shared_ptr<GameObjectBehavior> const & behavior,
	glm::mat4 const & relativeTransform,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader,
	VkCommandPool commandPool,
	std::string const & name) {
	auto constructedGameObject = GameObjectCreator::CreateMeshGameObject(
		material, baseModel, behavior,
		relativeTransform, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool,
		name);
	AddChildGameObject(std::static_pointer_cast<GameObject>
		(constructedGameObject));
}
