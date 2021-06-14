#include "DescriptorSetFunctions.h"
#include "LogicalDeviceManager.h"
#include "Resources/TextureCreator.h"
#include "Resources/Material.h"
#include <array>
#include <stdexcept>

VkDescriptorSetLayout DescriptorSetFunctions::CreateDescriptorSetLayout(VkDevice device,
												MaterialType materialType) {
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	switch (materialType) {
		case MaterialType::UnlitColor:
			descriptorSetLayout = CreateUnlitColorDescriptorSetLayout(device);
			break;
		case MaterialType::Text:
			descriptorSetLayout = CreateTextDescriptorSetLayout(device);
			break;
		case MaterialType::UnlitTintedTextured:
			descriptorSetLayout = CreateUnlitTintedTexturedDescriptorSetLayout(
				device);
			break;
		case MaterialType::MotherShip:
			descriptorSetLayout = CreateUnlitTintedTexturedDescriptorSetLayout(
				device);
			break;
		case MaterialType::WavySurface:
			descriptorSetLayout = CreateWavySurfaceDescriptorSetLayout(device);
			break;
		case MaterialType::BumpySurface:
			descriptorSetLayout = CreateBumpySurfaceDescriptorSetLayout(device);
			break;
		default:
			descriptorSetLayout = VK_NULL_HANDLE;
			break;
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSet(VkDevice device,
												std::shared_ptr<Material> const & material,
												VkDescriptorSet descriptorSet,
												VkDescriptorBufferInfo* bufferInfoVert,
												VkDescriptorBufferInfo* bufferInfoFrag) {
	auto materialType = material->GetMaterialType();
	auto textureCreator = material->GetTextureLoader();
	glm::vec4 tintColor(1.0f, 1.0f, 1.0f, 1.0f);

	switch (materialType) {
		case MaterialType::UnlitColor:
			tintColor = material->GetVec4("tint_color");
			UpdateDescriptorSetUnlitColor(device, descriptorSet, tintColor,
				bufferInfoVert, bufferInfoFrag);
			break;
		case MaterialType::Text:
			tintColor = material->GetVec4("tint_color");
			UpdateDescriptorSetText(device, descriptorSet, tintColor,
				textureCreator->GetTextureImageView(),
				textureCreator->GetTextureImageSampler(),
				bufferInfoVert, bufferInfoFrag);
			break;
		case MaterialType::UnlitTintedTextured:
			UpdateDescriptorSetUnlitTintedTextured(device, descriptorSet,
				textureCreator->GetTextureImageView(),
				textureCreator->GetTextureImageSampler(),
				bufferInfoVert);
			break;
		case MaterialType::MotherShip:
			UpdateDescriptorSetUnlitTintedTextured(device, descriptorSet,
				textureCreator->GetTextureImageView(),
				textureCreator->GetTextureImageSampler(),
				bufferInfoVert);
			break;
		case MaterialType::WavySurface:
			UpdateDescriptorSetWavySurface(device, descriptorSet,
				textureCreator->GetTextureImageView(),
				textureCreator->GetTextureImageSampler(),
				bufferInfoVert);
			break;
		case MaterialType::BumpySurface:
			UpdateDescriptorSetBumpySurface(device, descriptorSet,
				textureCreator->GetTextureImageView(),
				textureCreator->GetTextureImageSampler(),
				bufferInfoVert);
			break;
		default:
			descriptorSet = VK_NULL_HANDLE;
			break;
	}
}

VkDescriptorPool DescriptorSetFunctions::CreateDescriptorPool(VkDevice device, MaterialType materialType,
												  size_t numSwapChainImages) {
	VkDescriptorPool descriptorPool = nullptr;
	switch (materialType) {
		case MaterialType::UnlitColor:
			descriptorPool = CreateDescriptorPoolUnlitColor(device,
				numSwapChainImages);
			break;
		case MaterialType::Text:
			descriptorPool = CreateDescriptorPoolText(device,
				numSwapChainImages);
			break;
		case MaterialType::UnlitTintedTextured:
			descriptorPool = CreateDescriptorPoolUnlitTintedTextured(device,
																numSwapChainImages);
			break;
		case MaterialType::MotherShip:
			descriptorPool = CreateDescriptorPoolUnlitTintedTextured(device,
				numSwapChainImages);
			break;
		case MaterialType::WavySurface:
			descriptorPool = CreateDescriptorPoolWavySurface(device,
															numSwapChainImages);
			break;
		case MaterialType::BumpySurface:
			descriptorPool = CreateDescriptorPoolBumpySurface(device,
				numSwapChainImages);
			break;
		default:
			descriptorPool = VK_NULL_HANDLE;
			break;
	}
	return descriptorPool;
}

VkDescriptorSetLayout DescriptorSetFunctions::CreateTextDescriptorSetLayout(VkDevice device) {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBindingFrag = {};
	samplerLayoutBindingFrag.binding = 1;
	samplerLayoutBindingFrag.descriptorCount = 1;
	samplerLayoutBindingFrag.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBindingFrag.pImmutableSamplers = nullptr;
	samplerLayoutBindingFrag.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding uboLayoutBindingFrag = {};
	uboLayoutBindingFrag.binding = 2;
	uboLayoutBindingFrag.descriptorCount = 1;
	uboLayoutBindingFrag.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingFrag.pImmutableSamplers = nullptr;
	uboLayoutBindingFrag.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
		uboLayoutBinding, samplerLayoutBindingFrag, uboLayoutBindingFrag
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
		&descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout \
			for text!");
	}

	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetText(VkDevice device,
	VkDescriptorSet descriptorSet,
	glm::vec4 const& tintColor,
	VkImageView textureImageView,
	VkSampler textureSampler,
	VkDescriptorBufferInfo* bufferInfoVert,
	VkDescriptorBufferInfo* bufferInfoFrag) {
	std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = bufferInfoVert;

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureImageView;
	imageInfo.sampler = textureSampler;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pBufferInfo = bufferInfoFrag;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0, nullptr);
}

VkDescriptorPool DescriptorSetFunctions::CreateDescriptorPoolText(VkDevice device,
	size_t numSwapChainImages) {
	std::array<VkDescriptorPoolSize, 3> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr,
		&descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create text descriptor pool!");
	}
	return descriptorPool;
}

VkDescriptorSetLayout DescriptorSetFunctions::CreateUnlitColorDescriptorSetLayout(
	VkDevice device) {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding uboLayoutBindingFrag = {};
	uboLayoutBindingFrag.binding = 1;
	uboLayoutBindingFrag.descriptorCount = 1;
	uboLayoutBindingFrag.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingFrag.pImmutableSamplers = nullptr;
	uboLayoutBindingFrag.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
		uboLayoutBinding, uboLayoutBindingFrag
	};
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
		&descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout \
			for unlit color!");
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetUnlitColor(VkDevice device,
														VkDescriptorSet descriptorSet,
														glm::vec4 const& tintColor,
														VkDescriptorBufferInfo* bufferInfoVert,
														VkDescriptorBufferInfo* bufferInfoFrag) {
	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = bufferInfoVert;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = bufferInfoFrag;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0, nullptr);
}

VkDescriptorPool DescriptorSetFunctions::CreateDescriptorPoolUnlitColor(
	VkDevice device,
	size_t numSwapChainImages) {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool)
		!= VK_SUCCESS ){
		throw std::runtime_error("Failed to create unlit tinted textured descriptor pool!");
	}
	return descriptorPool;
}

VkDescriptorSetLayout DescriptorSetFunctions::
	CreateUnlitTintedTexturedDescriptorSetLayout(VkDevice device) {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	std::array<VkDescriptorSetLayoutBinding, 2> bindings =
		{ uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	
	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) !=
		VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout for unlit tinted textured!");
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetUnlitTintedTextured(VkDevice device,
											VkDescriptorSet descriptorSet,
											VkImageView textureImageView,
											VkSampler textureSampler,
											VkDescriptorBufferInfo* bufferInfoVert) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = textureImageView;
	imageInfo.sampler = textureSampler;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = bufferInfoVert;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(), 0,
		nullptr);
}

VkDescriptorPool DescriptorSetFunctions::CreateDescriptorPoolUnlitTintedTextured(VkDevice device,
   size_t numSwapChainImages) {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create unlit tinted textured descriptor pool!");
	}
	
	return descriptorPool;
}

VkDescriptorSetLayout DescriptorSetFunctions::
	CreateWavySurfaceDescriptorSetLayout(VkDevice device) {
	VkDescriptorSetLayoutBinding uboLayoutBindingVertexShader = {};
	uboLayoutBindingVertexShader.binding = 0;
	uboLayoutBindingVertexShader.descriptorCount = 1;
	uboLayoutBindingVertexShader.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingVertexShader.pImmutableSamplers = nullptr;
	uboLayoutBindingVertexShader.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	std::array<VkDescriptorSetLayoutBinding, 2> bindings =
		{ uboLayoutBindingVertexShader, samplerLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	
	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout for wavy surface!");
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetWavySurface(VkDevice device,
										 VkDescriptorSet descriptorSet,
										 VkImageView textureImageView,
										 VkSampler textureSampler,
										 VkDescriptorBufferInfo *bufferInfoVert) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = textureSampler;
	imageInfo.imageView = textureImageView;
	
	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = bufferInfoVert;
	
	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;
	
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkDescriptorPool DescriptorSetFunctions::CreateDescriptorPoolWavySurface(
						VkDevice device, size_t numSwapChainImages) {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create wavy surface descriptor pool!");
	}
	
	return descriptorPool;
}

VkDescriptorSetLayout DescriptorSetFunctions::CreateBumpySurfaceDescriptorSetLayout(
	VkDevice device) {
	VkDescriptorSetLayoutBinding uboLayoutBindingVertexShader = {};
	uboLayoutBindingVertexShader.binding = 0;
	uboLayoutBindingVertexShader.descriptorCount = 1;
	uboLayoutBindingVertexShader.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingVertexShader.pImmutableSamplers = nullptr;
	uboLayoutBindingVertexShader.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings =
	{ uboLayoutBindingVertexShader, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
		&descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout for bumpy surface!");
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetBumpySurface(VkDevice device,
	VkDescriptorSet descriptorSet,
	VkImageView textureImageView,
	VkSampler textureSampler,
	VkDescriptorBufferInfo* bufferInfoVert) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = textureSampler;
	imageInfo.imageView = textureImageView;

	std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = bufferInfoVert;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

VkDescriptorPool DescriptorSetFunctions::CreateDescriptorPoolBumpySurface(
	VkDevice device, size_t numSwapChainImages) {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create bumpy surface descriptor pool!");
	}

	return descriptorPool;
}
