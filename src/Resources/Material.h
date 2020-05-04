#pragma once

#include "DescriptorSetFunctions.h"

class ImageTextureLoader;

class Material {
public:
	Material(std::shared_ptr<ImageTextureLoader>const &
			 texture, DescriptorSetFunctions::MaterialType material);
	
	ImageTextureLoader* GetTextureLoader()
	{
		return textureLoader.get();
	}
	
	DescriptorSetFunctions::MaterialType GetMaterialType() const
	{
		return materialType;
	}

private:
	std::shared_ptr<ImageTextureLoader> textureLoader;
	DescriptorSetFunctions::MaterialType materialType;
};
