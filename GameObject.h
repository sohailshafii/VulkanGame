#pragma once

#include <memory>

class Model;

// game object with its own transform
// class will grow over time to include other relevant meta data
class GameObject {
public:
	GameObject(std::shared_ptr<Model> model);
	
	std::shared_ptr<Model> GetModel() {
		return objModel;
	}
	
private:
	std::shared_ptr<Model> objModel;
};
