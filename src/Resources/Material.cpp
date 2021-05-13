#include "Resources/Material.h"
#include "Resources/TextureCreator.h"

Material::Material(std::shared_ptr<TextureCreator>const &
			texture, DescriptorSetFunctions::MaterialType material,
			nlohmann::json const& materialNode) : textureCreator(texture),
		materialType(material) {
}

Material::Material(DescriptorSetFunctions::MaterialType material,
	nlohmann::json const& materialNode) : textureCreator(nullptr), materialType(material) {
}