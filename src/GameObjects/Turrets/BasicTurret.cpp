
#include "BasicTurret.h"
#include "GameObjectCreationUtilFuncs.h"
#include "StationaryGameObjectBehavior.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/MeshGameObject.h"
#include "GameObjects/Turrets/BasicTurretBehavior.h"
#include "Math/CommonMath.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float BasicTurret::turretWidth = 0.6f;
const float BasicTurret::turretDepth = 0.6f;
const float BasicTurret::turretHeight = 0.6f;
const float BasicTurret::topRadius = 0.3f;

BasicTurret::BasicTurret(Scene* const scene,
	std::shared_ptr<BasicTurretBehavior> const& behavior,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader,
	VkCommandPool commandPool,
	glm::mat4 const& localToWorldTransform) : GameObject(behavior) {
	behavior->SetTurret(this);
	name = "BasicTurret";

	localTransform = localToWorldTransform;
	localToWorld = localToWorldTransform;
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
	glm::mat4 baseRelTransform(1.0f);
	baseRelTransform = glm::translate(baseRelTransform,
		baseUpVec * 0.5f);
	turretBase = AddSubMeshAndReturnGameObject(baseMaterial, baseModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		baseRelTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool, "turretBase", this);

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
	turretBody = AddSubMeshAndReturnGameObject(bodyMaterial, bodyModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		boxRelativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool, "turretBody", this);

	// top of turret
	auto turretTopModel = Model::CreateIcosahedron(topRadius, 2);
	metadataNode = {
		{"tint_color",{0.0f, 1.0f, 0.0f, 1.0f }}
	};
	auto topMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto topRelativeTransform = glm::mat4(1.0f);
	glm::vec3 topCenter = baseUpVec + boxUpVec * (1.0f + topRadius);
	topRelativeTransform = glm::translate(topRelativeTransform,
		topCenter);
	turretTop = AddSubMeshAndReturnGameObject(topMaterial, turretTopModel,
		std::make_shared<StationaryGameObjectBehavior>(scene),
		topRelativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool, "turretTop", this);

	float gunLength = GetGunLength();
	auto gunRightVec = glm::vec3(turretWidth * 0.08f, 0.0f, 0.0f);
	auto gunUpVec = glm::vec3(0.0f, turretHeight * 0.08f, 0.0f);
	auto gunForwardVec = glm::vec3(0.0f, 0.0f, gunLength);
	gunCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	auto gunModel = Model::CreateBox(gunCenter, gunRightVec,
		gunUpVec, gunForwardVec);
	metadataNode = {
		{"tint_color",{1.0f, 0.0f, 0.0f, 1.0f }}
	};
	auto gunMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto gunBehavior = std::make_shared<StationaryGameObjectBehavior>(scene);
	float azim = glm::radians(80.0f);
	float polar = glm::radians(90.0f);
	glm::mat4 gunRelativeTransform = GetTransformForSphericalCoords(azim, polar,
		currentLookPoint);
	turretGun = AddSubMeshAndReturnGameObject(gunMaterial, gunModel, gunBehavior,
		gunRelativeTransform, gfxDeviceManager,
		logicalDeviceManager, resourceLoader,
		commandPool, "turretGun", turretTop.get());
}

void BasicTurret::SetGunTransformForSphericalCoords(float azim, float polar) {
	auto newTransform = GetTransformForSphericalCoords(azim, polar, currentLookPoint);
	turretGun->SetLocalTransform(newTransform);
}

void BasicTurret::SetGunLookRotation(glm::vec3 const& lookAtPoint) {
	currentLookPoint = lookAtPoint;
	glm::mat4 newTransform = GetTransformLookAtPoint(lookAtPoint);
	turretGun->SetLocalTransform(newTransform);
}

std::shared_ptr<GameObject> BasicTurret::AddSubMeshAndReturnGameObject(
	std::shared_ptr<Material> const & material,
	std::shared_ptr<Model> const & baseModel,
	std::shared_ptr<GameObjectBehavior> const & behavior,
	glm::mat4 const & relativeTransform,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader,
	VkCommandPool commandPool,
	std::string const & name,
	GameObject* parent) {
	auto constructedGameObject = GameObjectCreator::CreateMeshGameObject(
		material, baseModel, behavior,
		relativeTransform, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool,
		name);
	auto returnedGameObject = std::static_pointer_cast<GameObject>
		(constructedGameObject);
	parent->AddChildGameObject(returnedGameObject);
	return returnedGameObject;
}

glm::mat4 BasicTurret::GetTransformLookAtPoint(glm::vec3 const& lookAtPoint) {
	glm::vec3 lookAt = glm::normalize(lookAtPoint - gunCenter);
	glm::vec3 right, up;
	CommonMath::CreateCoordinateSystemForLookDir(lookAt, up, right);
	glm::mat4 rotationM(1.0f);
	rotationM[0] = glm::vec4(right, 0.0f);
	rotationM[1] = glm::vec4(up, 0.0f);
	rotationM[2] = glm::vec4(lookAt, 0.0f);

	// rotate the gun so that it faces outwards
	float gunLength = GetGunLength();
	glm::mat4 gunRelativeTransform = glm::translate(
		glm::mat4(1.0f),
		lookAtPoint + lookAt * gunLength * 0.5f);
	gunRelativeTransform *= rotationM;
	return gunRelativeTransform;
}

glm::mat4 BasicTurret::GetTransformForSphericalCoords(float azim, float polar,
	glm::vec3 & lookAtPoint) {
	lookAtPoint = CommonMath::GetCartesianFromSphericalCoords(azim,
		polar, topRadius);
	glm::mat4 gunRelativeTransform = GetTransformLookAtPoint(lookAtPoint);

	return gunRelativeTransform;
}
