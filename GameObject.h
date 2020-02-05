#pragma once

#include <memory>
#include <vector>
#include "vulkan/vulkan.h"
#include <glm/glm.hpp>

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
	
	glm::mat4 GetModelTransform() const {
		return model;
	}
	
	void SetModelTransform(const glm::mat4& model) {
		this->model = model;
	}
	
	VkBuffer GetVertexBuffer() const {
		return vertexBuffer;
	}
	
	VkBuffer GetIndexBuffer() const {
		return indexBuffer;
	}
	
	void CreateCommandBuffers(GfxDeviceManager* gfxDeviceManager,
								size_t numSwapChainImages);
	
private:
	std::shared_ptr<Model> objModel;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	
	glm::mat4 model;
	
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	
	void CreateVertexBuffer(const std::vector<Vertex>& vertices,
							GfxDeviceManager *gfxDeviceManager,
							VkCommandPool commandPool);
	void CreateIndexBuffer(const std::vector<uint32_t>& indices,
						   GfxDeviceManager *gfxDeviceManager,
						   VkCommandPool commandPool);
	
	void CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
							  size_t numSwapChainImages);
	void CleanUpUniformBuffers();
};
