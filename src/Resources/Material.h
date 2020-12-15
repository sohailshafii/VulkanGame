#pragma once

#include <glm/glm.hpp>
#include "DescriptorSetFunctions.h"

class TextureCreator;

class Material {
public:
	Material(std::shared_ptr<TextureCreator>const & texture,
			DescriptorSetFunctions::MaterialType material,
			glm::vec4 const & tintColor = glm::vec4(0.0f));

	Material(DescriptorSetFunctions::MaterialType material,
			glm::vec4 const& tintColor = glm::vec4(0.0f));
	
	TextureCreator* GetTextureLoader() {
		return textureCreator.get();
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return materialType;
	}

	glm::vec4 GetTintColor() const {
		return tintColor;
	}
	
	void SetImageTextureLoader(std::shared_ptr<TextureCreator>const &
		texture) {
		textureCreator = texture;
	}
	
	void SetMaterialType(DescriptorSetFunctions::MaterialType
						 material) {
		materialType = material;
	}

	void SetTintColor(glm::vec4 const& tintColor) {
		this->tintColor = tintColor;
	}

private:
	std::shared_ptr<TextureCreator> textureCreator;
	DescriptorSetFunctions::MaterialType materialType;
	glm::vec4 tintColor;
};
