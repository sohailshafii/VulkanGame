#include "Resources/Material.h"
#include "Resources/ImageTextureLoader.h"

Material::Material(std::shared_ptr<ImageTextureLoader>const &
		 texture, DescriptorSetFunctions::MaterialType material)
: textureLoader(texture), materialType(material)
{
	
}
