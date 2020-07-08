#include "SceneManagement/Scene.h"
#include "ResourceLoader.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include <iostream>

// used for game scene

Scene::Scene() {
	// TODO initialize properly
	resourceLoader = nullptr;
	gfxDeviceManager = nullptr;
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
	std::cout << "hello world!\n";
}