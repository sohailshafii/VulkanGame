#include "SceneManagement/Scene.h"
#include "ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "GameObjectCreationUtilFuncs.h"
#include "PawnBehavior.h"
#include "GameObject.h"
#include "PlayerGameObjectBehavior.h"
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

void Scene::RemoveGameObject(GameObject* gameObjectToRemove) {
	int removalIndex = -1;
	for (int i = 0; i < gameObjects.size(); i++) {
		if (gameObjects[i].get() == gameObjectToRemove) {
			removalIndex = i;
			break;
		}
	}

	if (removalIndex != -1) {
		gameObjects.erase(gameObjects.begin() + removalIndex);
	}
}

void Scene::RemoveGameObject(std::shared_ptr<GameObject> const & gameObjectToRemove) {
	int removalIndex = -1;
	for (int i = 0; i < gameObjects.size(); i++) {
		if (gameObjects[i] == gameObjectToRemove) {
			removalIndex = i;
			break;
		}
	}

	if (removalIndex != -1) {
		gameObjects.erase(gameObjects.begin() + removalIndex);
	}
}

void Scene::RemoveGameObjects(
	std::vector<GameObject *> const& gameObjectsToRemove) {
	for (int i = 0; i < gameObjectsToRemove.size(); i++) {
		RemoveGameObject(gameObjectsToRemove[i]);
	}
}

void Scene::RemoveGameObjects(
	std::vector<std::shared_ptr<GameObject>> const& gameObjectsToRemove) {
	for (int i = 0; i < gameObjectsToRemove.size(); i++) {
		RemoveGameObject(gameObjectsToRemove[i]);
	}
}

std::shared_ptr<GameObject> Scene::GetPlayerGameObject() {
	std::shared_ptr<GameObject> foundPlayer = nullptr;

	for (std::shared_ptr<GameObject> playerObj : gameObjects) {
		auto objBehavior = playerObj->GetGameObjectBehavior();
		auto asPlayerBehav =
			dynamic_cast<PlayerGameObjectBehavior*>(objBehavior);
		if (asPlayerBehav != nullptr) {
			foundPlayer = playerObj;
			break;
		}
	}

	return foundPlayer;
}

void  Scene::SpawnGameObject(std::string const& gameObjectName,
	glm::vec3 spawnPosition) {
	std::shared_ptr gameObjectMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitTintedTextured,
		"texture.jpg", resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	std::shared_ptr gameObjectModel = GameObjectCreator::LoadModelFromName(
		"cube.obj", resourceLoader);
	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		spawnPosition);

	std::shared_ptr<GameObject> newGameObject =
		GameObjectCreator::CreateGameObject(gameObjectMaterial,
			gameObjectModel, std::make_unique<PawnBehavior>(this),
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
