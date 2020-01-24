#pragma once

#include <memory>
#include <vector>
#include "vulkan/vulkan.h"

class Model;
struct Vertex;
class GfxDeviceManager;
class LogicalDeviceManager;

// game object with its own transform
// class will grow over time to include other relevant meta data
class GameObject {
public:
	GameObject(std::shared_ptr<Model> model,
			   GfxDeviceManager *gfxDeviceManager,
			   std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
			   VkCommandPool commandPool);
	
	~GameObject();
	
	std::shared_ptr<Model> GetModel() {
		return objModel;
	}
	
	VkBuffer GetVertexBuffer() const {
		return vertexBuffer;
	}
	
	VkBuffer GetIndexBuffer() const {
		return indexBuffer;
	}
	
private:
	std::shared_ptr<Model> objModel;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	
	void createVertexBuffer(const std::vector<Vertex>& vertices,
							GfxDeviceManager *gfxDeviceManager,
							VkCommandPool commandPool);
	void createIndexBuffer(const std::vector<uint32_t>& indices,
						   GfxDeviceManager *gfxDeviceManager,
						   VkCommandPool commandPool);
};
