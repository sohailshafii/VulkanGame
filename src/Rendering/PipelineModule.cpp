#include "PipelineModule.h"
#include "ShaderLoader.h"
#include "Resources/ResourceLoader.h"
#include "Common.h"
#include "Vertex.h"
#include <iostream>

PipelineModule::PipelineModule(const std::string& vertShaderName,
	const std::string& fragShaderName, VkDevice device,
	VkExtent2D swapChainExtent, GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader,
	VkDescriptorSetLayout descriptorSetLayout,
	VkRenderPass renderPass,
	DescriptorSetFunctions::MaterialType materialType,
	VkPrimitiveTopology primitiveTopology) : device(device),
	materialType(materialType), primitiveTopology(primitiveTopology) {
#if __APPLE__
	std::shared_ptr<ShaderLoader> vertShaderModule = resourceLoader->GetShader(
	"../../shaders/" + vertShaderName, device);
std::shared_ptr<ShaderLoader> fragShaderModule = resourceLoader->GetShader(
	"../../shaders/" + fragShaderName, device);
#else
	std::shared_ptr<ShaderLoader> vertShaderModule = resourceLoader->GetShader(
		"../shaders/" + vertShaderName, device);
	std::shared_ptr<ShaderLoader> fragShaderModule = resourceLoader->GetShader(
		"../shaders/" + fragShaderName, device);

#endif
	
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule->GetVkShaderModule();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule->GetVkShaderModule();
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertShaderStageInfo, fragShaderStageInfo
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription bindingDescription;
	VkVertexInputAttributeDescription *attribDescriptionArray = nullptr;
	size_t numAttrib = 0;
	if (materialType == DescriptorSetFunctions::MaterialType::UnlitColor) {
		bindingDescription = VertexPos::GetBindingDescription();
		auto attributeDescriptions = VertexPos::GetAttributeDescriptions();
		numAttrib = attributeDescriptions.size();
		attribDescriptionArray = new VkVertexInputAttributeDescription[numAttrib];
		for (size_t i = 0; i < numAttrib; i++) {
			attribDescriptionArray[i] = attributeDescriptions[i];
		}
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::Text) {
		bindingDescription = VertexPosTex::GetBindingDescription();
		auto attributeDescriptions = VertexPosTex::GetAttributeDescriptions();
		numAttrib = attributeDescriptions.size();
		attribDescriptionArray = new VkVertexInputAttributeDescription[numAttrib];
		for (size_t i = 0; i < numAttrib; i++) {
			attribDescriptionArray[i] = attributeDescriptions[i];
		}
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::UnlitTintedTextured ||
		materialType == DescriptorSetFunctions::MaterialType::MotherShip) {
		bindingDescription = VertexPosColorTexCoord::GetBindingDescription();
		auto attributeDescriptions = VertexPosColorTexCoord::GetAttributeDescriptions();
		numAttrib = attributeDescriptions.size();
		attribDescriptionArray = new VkVertexInputAttributeDescription[
			numAttrib];
		for (size_t i = 0; i < numAttrib; i++) {
			attribDescriptionArray[i] = attributeDescriptions[i];
		}
	}
	else if (materialType == DescriptorSetFunctions::MaterialType::WavySurface ||
		materialType == DescriptorSetFunctions::MaterialType::BumpySurface) {
		bindingDescription = VertexPosNormalColorTexCoord::GetBindingDescription();
		auto attributeDescriptions = VertexPosNormalColorTexCoord::GetAttributeDescriptions();
		numAttrib = attributeDescriptions.size();
		attribDescriptionArray = new VkVertexInputAttributeDescription[
			numAttrib];
		for (size_t i = 0; i < numAttrib; i++) {
			attribDescriptionArray[i] = attributeDescriptions[i];
		}
	}
	
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)numAttrib;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attribDescriptionArray;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = primitiveTopology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	bool usePolygonLineMode =
		materialType == DescriptorSetFunctions::MaterialType::MotherShip ||
		materialType == DescriptorSetFunctions::MaterialType::BumpySurface ||
		materialType == DescriptorSetFunctions::MaterialType::WavySurface ||
		materialType == DescriptorSetFunctions::MaterialType::UnlitTintedTextured ||
		materialType == DescriptorSetFunctions::MaterialType::UnlitColor;
	rasterizer.polygonMode = usePolygonLineMode ? VK_POLYGON_MODE_LINE :
		VK_POLYGON_MODE_FILL;
	// TODO: revert width code. also need a device feature store to see what we can use (see LogicalDeviceManager)
	rasterizer.lineWidth = 2.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // optional
	rasterizer.depthBiasClamp = 0.0f; // optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // optional

	VkPipelineMultisampleStateCreateInfo multiSampling = {};
	multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampling.sampleShadingEnable = VK_TRUE;
	multiSampling.rasterizationSamples = gfxDeviceManager->GetMSAASamples();
	multiSampling.minSampleShading = 0.2f; // min fraction for sample shading; 
	// closer to one is smoother
	multiSampling.pSampleMask = nullptr;
	multiSampling.alphaToCoverageEnable = VK_FALSE;
	multiSampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = 
		SpecifyBlendStateForMaterial(materialType);

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // optional
	colorBlending.blendConstants[1] = 0.0f; // optional
	colorBlending.blendConstants[2] = 0.0f; // optional
	colorBlending.blendConstants[3] = 0.0f; // optional

	/*VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;*/

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil =
		SpecifyDepthStencilStateForMaterial(materialType);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multiSampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
		&pipelineInfo, nullptr, &graphicsPipeline);
	std::cout << "Pipeline result " << result << std::endl;
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}
	
	if (attribDescriptionArray != nullptr) {
		delete [] attribDescriptionArray;
	}
}

PipelineModule::~PipelineModule() {
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

VkPipelineColorBlendAttachmentState PipelineModule::SpecifyBlendStateForMaterial(
	DescriptorSetFunctions::MaterialType materialType) {
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (materialType == DescriptorSetFunctions::MaterialType::Text) {
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	else {
		colorBlendAttachment.blendEnable = VK_FALSE;
		// rest are optional (for now)
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	return colorBlendAttachment;
}

VkPipelineDepthStencilStateCreateInfo PipelineModule::SpecifyDepthStencilStateForMaterial(
	DescriptorSetFunctions::MaterialType materialType) {
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = materialType == DescriptorSetFunctions::MaterialType::Text ?
		VK_FALSE : VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // optional
	depthStencil.back = {}; // optional

	return depthStencil;
}

