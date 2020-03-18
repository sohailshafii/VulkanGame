#include "DescriptorSetFunctions.h"
#include <array>
#include <stdexcept>

VkDescriptorSetLayout DescriptorSetFunctions::CreateDescriptorSetLayout(VkDevice device,
												MaterialType materialType) {
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	switch (materialType) {
		case MaterialType::UnlitTintedTextured:
			descriptorSetLayout = CreateUnlitTintedTexturedDescriptorSetLayout(device);
			break;
		case MaterialType::SimpleLambertian:
			descriptorSetLayout = CreateSimpleLambertianDescriptorSetLayout(device);
			break;
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSet(VkDevice device,
												 MaterialType materialType,
												 VkDescriptorSet descriptorSet,
												 VkImageView textureImageView,
												 VkSampler textureSampler,
												 VkDescriptorBufferInfo* bufferInfoVert,
												 VkDescriptorBufferInfo* bufferInfoFrag) {
	switch (materialType) {
		case MaterialType::UnlitTintedTextured:
			UpdateDescriptorSetUnlitTintedTextured(device, descriptorSet,
												   textureImageView, textureSampler,
												   bufferInfoVert);
			break;
		case MaterialType::SimpleLambertian:
			UpdateDescriptorSetSimpleLambertian(device, descriptorSet, textureImageView, textureSampler, bufferInfoVert, bufferInfoFrag);
			break;
	}
}

VkDescriptorSetLayout DescriptorSetFunctions::CreateUnlitTintedTexturedDescriptorSetLayout(VkDevice device)
{
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
		VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetUnlitTintedTextured(VkDevice device,
											VkDescriptorSet descriptorSet,
											VkImageView textureImageView,
											VkSampler textureSampler,
											VkDescriptorBufferInfo* bufferInfoVert)
{
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

VkDescriptorSetLayout DescriptorSetFunctions::CreateSimpleLambertianDescriptorSetLayout(VkDevice device)
{
	VkDescriptorSetLayoutBinding uboLayoutBindingVertexShader = {};
	uboLayoutBindingVertexShader.binding = 0;
	uboLayoutBindingVertexShader.descriptorCount = 1;
	uboLayoutBindingVertexShader.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingVertexShader.pImmutableSamplers = nullptr;
	uboLayoutBindingVertexShader.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	VkDescriptorSetLayoutBinding fragmentUniformBuffer = {};
	fragmentUniformBuffer.binding = 2;
	fragmentUniformBuffer.descriptorCount = 1;
	fragmentUniformBuffer.descriptorCount = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	fragmentUniformBuffer.pImmutableSamplers = nullptr;
	fragmentUniformBuffer.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	std::array<VkDescriptorSetLayoutBinding, 3> bindings =
		{ uboLayoutBindingVertexShader, samplerLayoutBinding, fragmentUniformBuffer};
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();
	
	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout)
		!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor set layout for lambertian!");
	}
	return descriptorSetLayout;
}

void DescriptorSetFunctions::UpdateDescriptorSetSimpleLambertian(VkDevice device,
										 VkDescriptorSet descriptorSet,
										 VkImageView textureImageView,
										 VkSampler textureSampler,
										 VkDescriptorBufferInfo *bufferInfoVert,
										 VkDescriptorBufferInfo *bufferInfoFrag)
{
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = textureSampler;
	
	std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
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
	
	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 2;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pBufferInfo = bufferInfoFrag;
	
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}