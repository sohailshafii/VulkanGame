#pragma once

#include <glm/glm.hpp>
#include "DescriptorSetFunctions.h"

class ImageTextureLoader;

class Material {
public:
	Material(std::shared_ptr<ImageTextureLoader>const & texture,
			DescriptorSetFunctions::MaterialType material,
			glm::vec4 const & tintColor = glm::vec4(0.0f));
	
	ImageTextureLoader* GetTextureLoader() {
		return textureLoader.get();
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return materialType;
	}

	glm::vec4 GetTintColor() const {
		return tintColor;
	}
	
	void SetImageTextureLoader(std::shared_ptr<ImageTextureLoader>const &
							   texture) {
		textureLoader = texture;
	}
	
	void SetMaterialType(DescriptorSetFunctions::MaterialType
						 material) {
		materialType = material;
	}

	void SetTintColor(glm::vec4 const& tintColor) {
		this->tintColor = tintColor;
	}

private:
	std::shared_ptr<ImageTextureLoader> textureLoader;
	DescriptorSetFunctions::MaterialType materialType;
	glm::vec4 tintColor;
};
