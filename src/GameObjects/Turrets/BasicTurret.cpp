
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
	float const turretWidth = 0.75f;
	float const turretDepth = 0.75f;
	float const turretHeight = 0.75f;

	// base of turret
	auto baseCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	auto baseRightVec = glm::vec3(turretWidth, 0.0f, 0.0f);
	auto baseUpVec = glm::vec3(0.0f, turretHeight*0.25f, 0.0f);
	auto baseForwardVec = glm::vec3(0.0f, 0.0f, turretDepth);
	auto baseModel = Model::CreateBox(baseCenter, baseRightVec,
		baseUpVec, baseForwardVec);
	nlohmann::json metadataNode = {
		{"tint_color",{0.0f, 1.0f, 0.0f, 1.0f }}
	};
	auto baseMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto baseBehavior = std::make_shared<StationaryGameObjectBehavior>(scene);
	glm::mat4 baseRelTransform(1.0f);
	baseRelTransform = glm::translate(baseRelTransform,
		baseUpVec * 0.5f);
	AddSubMesh(baseMaterial, baseModel, baseBehavior,
		baseRelTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool,
		"turretBase");

	// body of turret
	auto boxRightVec = glm::vec3(turretWidth, 0.0f, 0.0f);
	auto boxUpVec =  glm::vec3(0.0f, turretHeight, 0.0f);
	auto boxForwardVec = glm::vec3(0.0f, 0.0f, turretDepth);
	auto boxCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	auto bodyModel = Model::CreateBox(boxCenter, boxRightVec,
		boxUpVec, boxForwardVec);
	name = "BasicTurret";
	metadataNode = {
		{"tint_color",{1.0f, 0.0f, 0.0f, 1.0f }}
	};
	auto bodyMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto boxRelativeTransform = glm::mat4(1.0f);
	boxRelativeTransform = glm::translate(boxRelativeTransform,
		baseUpVec + boxUpVec * 0.5f);
	AddSubMesh(bodyMaterial, bodyModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		boxRelativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool,
		"turretBody");

	// top of turret
	float topRadius = 0.3f;
	auto turretTopModel = Model::CreateIcosahedron(topRadius, 2);
	metadataNode = {
		{"tint_color",{0.0f, 1.0f, 0.0f, 1.0f }}
	};
	auto topMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto topRelativeTransform = glm::mat4(1.0f);
	topRelativeTransform = glm::translate(topRelativeTransform,
		baseUpVec + boxUpVec * (1.0f + topRadius));
	AddSubMesh(topMaterial, turretTopModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		topRelativeTransform, gfxDeviceManager,
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
