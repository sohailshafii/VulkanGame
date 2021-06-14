#pragma once

#include <glm/glm.hpp>
#include "DescriptorSetFunctions.h"
#include "nlohmann/json.hpp"
#include "Common.h"
#include <iostream>

class TextureCreator;

class Material {
public:
	Material(std::shared_ptr<TextureCreator>const & texture,
			DescriptorSetFunctions::MaterialType material,
			nlohmann::json const & materialNode);

	Material(DescriptorSetFunctions::MaterialType material,
		nlohmann::json const& materialNode = nlohmann::json());
	
	TextureCreator* GetTextureLoader() {
		return textureCreator.get();
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return materialType;
	}

	glm::vec4 GetVec4(std::string propertyName) const {
		std::cout << materialNode << std::endl;
		auto tintColorObj = Common::SafeGetToken(materialNode, propertyName);
		return glm::vec4((float)tintColorObj[0], (float)tintColorObj[1],
			(float)tintColorObj[2], (float)tintColorObj[3]);
	}
	
	void SetImageTextureLoader(std::shared_ptr<TextureCreator>const &
		texture) {
		textureCreator = texture;
	}
	
	void SetMaterialType(DescriptorSetFunctions::MaterialType
						 material) {
		materialType = material;
	}

private:
	std::shared_ptr<TextureCreator> textureCreator;
	DescriptorSetFunctions::MaterialType materialType;

	nlohmann::json materialNode;
};
