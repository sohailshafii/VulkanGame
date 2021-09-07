#pragma once

#include <glm/glm.hpp>
#include "SceneManagement/Scene.h"

class GameObject;

class GameObjectBehavior {
public:
	enum class BehaviorStatus : char { Normal = 0, Destroyed };

	GameObjectBehavior(Scene * scene)
		: scene(scene), gameObject(nullptr) {
	}

	GameObjectBehavior()
		: scene(nullptr), gameObject(nullptr) {
	}
	
	virtual ~GameObjectBehavior() {
	}

	virtual BehaviorStatus UpdateSelf(float time, float deltaTime) = 0;

	void SetScene(Scene* scene) {
		this->scene = scene;
	}

	void SetGameObject(GameObject* gameObject) {
		this->gameObject = gameObject;
	}

	GameObject* GetGameObject() {
		return gameObject;
	}

protected:
	// we don't own this pointer; should be shared_ptr ideally?
	// the problem is that we want to de-allocate objects in a certain order
	// because an object relies on a vulkan logic device, the latter of which
	// should be deleted after. Need to enforce proper destruction order somehow
	// with shared pointers as opposed to using classical pointers.
	Scene * scene;
	GameObject* gameObject;
};
