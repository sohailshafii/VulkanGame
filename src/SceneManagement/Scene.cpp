#include "SceneManagement/Scene.h"
#include "ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "GameObjectCreationUtilFuncs.h"
#include "PawnBehavior.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Scene::Scene(ResourceLoader* resourceLoader,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const & logicalDeviceManager,
	VkCommandPool commandPool) : resourceLoader(resourceLoader),
		gfxDeviceManager(gfxDeviceManager),
		logicalDeviceManager(logicalDeviceManager), commandPool(commandPool) {
}

Scene::~Scene() {
}

void Scene::AddGameObject(std::shared_ptr<GameObject>
						  const & newGameObject) {
	gameObjects.push_back(newGameObject);
}

GameObject* Scene::GetGameObject(unsigned int index) {
	if (index >= gameObjects.size()) {
		return nullptr;
	}
	return gameObjects[index].get();
}

std::shared_ptr<GameObject> Scene::NewGameObject() {
	return nullptr;
}

void  Scene::SpawnGameObject(std::string const& gameObjectName) {
	std::shared_ptr gameObjectMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitTintedTextured,
		"texture.jpg", resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	// TODO: debug model loading
	/*std::shared_ptr gameObjectModel = resourceLoader->GetModel("cube");
	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 0.0f, 4.0f));

	std::shared_ptr<GameObject> newGameObject =
		GameObjectCreator::CreateGameObject(gameObjectMaterial,
			gameObjectModel, std::make_unique<PawnBehavior>(),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	AddGameObject(newGameObject);*/
	std::cout << "spawned pawn!\n";
}