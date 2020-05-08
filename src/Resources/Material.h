#pragma once

#include "DescriptorSetFunctions.h"

class ImageTextureLoader;

class Material {
public:
	Material(std::shared_ptr<ImageTextureLoader>const &
			 texture, DescriptorSetFunctions::MaterialType material);
	
	ImageTextureLoader* GetTextureLoader() {
		return textureLoader.get();
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const {
		return materialType;
	}
	
	void SetImageTextureLoader(std::shared_ptr<ImageTextureLoader>const &
							   texture) {
		textureLoader = texture;
	}
	
	void SetMaterialType(DescriptorSetFunctions::MaterialType
						 material) {
		materialType = material;
	}

private:
	std::shared_ptr<ImageTextureLoader> textureLoader;
	DescriptorSetFunctions::MaterialType materialType;
};
