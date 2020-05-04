#include "Scene.h"

// used for game scene

Scene::Scene() {
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
