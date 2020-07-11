#include "SceneManagement/Scene.h"
#include "ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "GameObjectCreationUtilFuncs.h"
#include "PawnBehavior.h"
#include "GameObject.h"
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

void  Scene::SpawnGameObject(std::string const& gameObjectName) {
	std::shared_ptr gameObjectMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitTintedTextured,
		"texture.jpg", resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	std::shared_ptr gameObjectModel = GameObjectCreator::LoadModelFromName(
		"cube.obj", resourceLoader);
	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 0.0f, 4.0f));

	std::shared_ptr<GameObject> newGameObject =
		GameObjectCreator::CreateGameObject(gameObjectMaterial,
			gameObjectModel, std::make_unique<PawnBehavior>(),
			localToWorldTransform, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	upcomingGameObjects.push_back(newGameObject);
}

void Scene::Update(float time, float deltaTime, uint32_t imageIndex,
	glm::mat4 const & viewMatrix, VkExtent2D swapChainExtent) {
	for (auto& gameObject : upcomingGameObjects) {
		gameObjects.push_back(gameObject);
	}
	upcomingGameObjects.clear();

	for (std::shared_ptr<GameObject>& gameObject : gameObjects) {
		if (!gameObject->GetInitializedInEngine()) {
			continue;
		}
		gameObject->UpdateState(time, deltaTime);
		gameObject->UpdateVisualState(imageIndex,
			viewMatrix, time, deltaTime, swapChainExtent);
	}
}