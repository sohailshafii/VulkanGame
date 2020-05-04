#pragma once

#include <memory>
#include <vector>

class GameObject;

class Scene
{
public:
	Scene();
	~Scene();
	
	void AddGameObject(std::shared_ptr<GameObject> const & newGameObject);
	
	GameObject* GetGameObject(unsigned int index);
	
private:
	std::vector<std::shared_ptr<GameObject>> gameObjects;
	
};

