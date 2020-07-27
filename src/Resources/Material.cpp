#include "Resources/Material.h"
#include "Resources/ImageTextureLoader.h"

Material::Material(std::shared_ptr<ImageTextureLoader>const &
		 texture, DescriptorSetFunctions::MaterialType material,
		glm::vec4 const& tintColor)
: textureLoader(texture), materialType(material), tintColor(tintColor)
{
	
}
