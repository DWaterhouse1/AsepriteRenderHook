#include "Pipeline.h"

// std
#include <fstream>
#include <iostream>
#include <cassert>

namespace wrengine
{
Pipeline::Pipeline(
	Device& device,
	const std::string& vertFilepath,
	const std::string& fragFilepath,
	const PipelineConfigInfo& pipelineInfo) :
	m_device{ device }
{
	createGraphicsPipeline(vertFilepath, fragFilepath, pipelineInfo);
}

Pipeline::~Pipeline()
{
	vkDestroyShaderModule(m_device.device(), m_vertShaderModule, nullptr);
	vkDestroyShaderModule(m_device.device(), m_fragShaderModule, nullptr);
	vkDestroyPipeline(m_device.device(), m_grahpicsPipeline, nullptr);
}

/**
* Reads a file as binary into a char vector. Will throw a runtime error if
* the file cannot be opened.
* 
* @param filepath Path of the file to read.
* 
* @return Data read from the file.
*/
std::vector<char> Pipeline::readFile(const std::string& filepath)
{
	std::ifstream file{ filepath, std::ios::ate | std::ios::binary };
	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + filepath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

/**
* Creates and constructs the graphics pipeline object. Reads SPIR-V shader files
* and compiles into shader program. Will throw a runtime error on failure to
* create graphics pipeline.
* 
* @param vertFilepath Path to the SPIR-V vertex shader code.
* @param fragFilepath Path to the SPIR-V fragment shader code.
* @param pipelineInfo Pipeline configuration info strucutre.
*/
void Pipeline::createGraphicsPipeline(
	const std::string& vertFilepath,
	const std::string& fragFilepath,
	const PipelineConfigInfo& pipelineInfo
)
{
	assert(pipelineInfo.pipelineLayout != VK_NULL_HANDLE &&
		"Cannot create graphics pipeline - no pipeline layout!");
	std::vector<char> vertCode = readFile(vertFilepath);
	std::vector<char> fragCode = readFile(fragFilepath);

	createShaderModule(vertCode, &m_vertShaderModule);
	createShaderModule(fragCode, &m_fragShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2]{};
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = m_vertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = m_fragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	
	std::vector<VkVertexInputBindingDescription> bindingDescriptions =
		Model::Vertex::getBindingDescriptions();
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
		Model::Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount =
		static_cast<uint32_t>(bindingDescriptions.size());;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &pipelineInfo.inputAssemblyInfo;
	pipelineCreateInfo.pViewportState = &pipelineInfo.viewportInfo;
	pipelineCreateInfo.pRasterizationState = &pipelineInfo.rasterizationInfo;
	pipelineCreateInfo.pMultisampleState = &pipelineInfo.multisampleInfo;
	pipelineCreateInfo.pColorBlendState = &pipelineInfo.colorBlendInfo;
	pipelineCreateInfo.pDynamicState = &pipelineInfo.dynamicStateInfo;
	pipelineCreateInfo.pDepthStencilState = &pipelineInfo.depthStencilInfo;

	pipelineCreateInfo.layout = pipelineInfo.pipelineLayout;
	pipelineCreateInfo.renderPass = pipelineInfo.renderPass;
	pipelineCreateInfo.subpass = pipelineInfo.subpass;

	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	
	if (vkCreateGraphicsPipelines(
		m_device.device(),
		VK_NULL_HANDLE,
		1,
		&pipelineCreateInfo,
		nullptr,
		&m_grahpicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("unable to create graphics pipelines!");
	}
}

/**
* Creates shader module from supplied SPIR-V shader code. Will throw a runtime
* error if shader module cannot be created.
* 
* @param code The code to compile into the shader module.
* @param shaderModule Pointer to the created shader module.
*/
void Pipeline::createShaderModule(
	const std::vector<char>& code,
	VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(
		m_device.device(),
		&createInfo,
		nullptr,
		shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
}

/**
* Submits a call to bind the pipeline ready for draw.
* 
* @param commandBuffer The command buffer into which the bind command is
* recorded.
*/
void Pipeline::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_grahpicsPipeline);
}

/**
* In place sets default values for a PipelineConfigInfo structure.
* 
* @param configInfo Config info structure to set.
*/
void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.inputAssemblyInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.inputAssemblyInfo.topology =
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.viewportInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.viewportInfo.viewportCount = 1;
	configInfo.viewportInfo.pViewports = nullptr;
	configInfo.viewportInfo.scissorCount = 1;
	configInfo.viewportInfo.pScissors = nullptr;

	configInfo.rasterizationInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.rasterizationInfo.lineWidth = 1.0f;
	configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
	configInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
	configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

	configInfo.multisampleInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	configInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
	configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
	configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
	configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

	configInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
	configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

	configInfo.colorBlendInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
	configInfo.colorBlendInfo.attachmentCount = 1;
	configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
	configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
	configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
	configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
	configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

	configInfo.depthStencilInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
	configInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
	configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.depthStencilInfo.front = {};  // Optional
	configInfo.depthStencilInfo.back = {};   // Optional

	configInfo.dynamicStateEnables =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	configInfo.dynamicStateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.dynamicStateInfo.pDynamicStates =
		configInfo.dynamicStateEnables.data();
	configInfo.dynamicStateInfo.dynamicStateCount =
		static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
	configInfo.dynamicStateInfo.flags = 0;
}
} // namespace wrengine