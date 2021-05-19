#include "Resources/Material.h"
#include "Resources/TextureCreator.h"

Material::Material(std::shared_ptr<TextureCreator>const &
			texture, DescriptorSetFunctions::MaterialType material,
			nlohmann::json const& materialNode) : textureCreator(texture),
		materialType(material), materialNode(materialNode) {
}

Material::Material(DescriptorSetFunctions::MaterialType material,
	glm::vec4 const& tintColor) : textureCreator(nullptr),
	materialType(material), tintColor(tintColor) {
}