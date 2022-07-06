#include "vulkan_pipelines.h"

#include "vulkan_commands.h"

#include <stdexcept>

namespace vkimpl {

/**
* The implementation of class VulkanImages
*/


//--------------------------------------------------------------------------------------------------
// Create the graphics pipeline given the shader codes and some settings
//
void VulkanPipeline::initAndCreateGraphicsPipeline(VulkanPipelineCreateInfo info, VkPipelineLayout& graphicsPipelineLayout, VkPipeline& graphicsPipeline) {
	initGraphicsPipelineCreateInfo(info);
	createGraphicsPipelineLayout(graphicsPipelineLayout);
	createGraphicsPipeline(graphicsPipeline);
}

//--------------------------------------------------------------------------------------------------
// Create the graphics pipeline create info
//
void VulkanPipeline::initGraphicsPipelineCreateInfo(VulkanPipelineCreateInfo info) {
	m_vulkanPipelineCreateInfo = info;
	populateShaderStages();
	populateVertexInput();
	populateViewPoint();
	populateRasterization();
	populateMultisampling();
	populateDepthStencil();
	populateColorBlend();
}


//--------------------------------------------------------------------------------------------------
// Populate the rasterization information
//
void VulkanPipeline::createGraphicsPipelineLayout(VkPipelineLayout& graphicsPipelineLayout) {
	m_graphicsPipelineLayoutInfo = {};
	m_graphicsPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	m_graphicsPipelineLayoutInfo.setLayoutCount = m_vulkanPipelineCreateInfo.descriptorSetLayouts.size();
	m_graphicsPipelineLayoutInfo.pSetLayouts = m_vulkanPipelineCreateInfo.descriptorSetLayouts.data();
	m_graphicsPipelineLayoutInfo.pushConstantRangeCount = 0;
	m_graphicsPipelineLayoutInfo.pPushConstantRanges = nullptr; 

	if (vkCreatePipelineLayout(m_device, &m_graphicsPipelineLayoutInfo, nullptr, &m_graphicsPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	graphicsPipelineLayout = m_graphicsPipelineLayout;
}

//--------------------------------------------------------------------------------------------------
// Create the graphics pipeline and set the pipelien layout and pipeline values
//
void VulkanPipeline::createGraphicsPipeline(VkPipeline& graphicsPipeline) {
	m_graphicsPipelineCreateInfo = {};
	m_graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	m_graphicsPipelineCreateInfo.stageCount = m_shaderStages.size();
	m_graphicsPipelineCreateInfo.pStages = m_shaderStages.data();
	m_graphicsPipelineCreateInfo.pVertexInputState = &m_vertexInputInfo;
	m_graphicsPipelineCreateInfo.pInputAssemblyState = &m_inputAssemblyInfo;
	m_graphicsPipelineCreateInfo.pViewportState = &m_viewportStateInfo;
	m_graphicsPipelineCreateInfo.pRasterizationState = &m_rasterizationInfo;
	m_graphicsPipelineCreateInfo.pMultisampleState = &m_multisamplingInfo;
	m_graphicsPipelineCreateInfo.pDepthStencilState = &m_depthStencilInfo;
	m_graphicsPipelineCreateInfo.pColorBlendState = &m_colorBlendingInfo;
	m_graphicsPipelineCreateInfo.layout = m_graphicsPipelineLayout;
	m_graphicsPipelineCreateInfo.renderPass = m_vulkanPipelineCreateInfo.renderPass;
	m_graphicsPipelineCreateInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &m_graphicsPipelineCreateInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	graphicsPipeline = m_graphicsPipeline;

	if (m_fragShaderModule != VK_NULL_HANDLE)
		vkDestroyShaderModule(m_device, m_fragShaderModule, nullptr);
	if (m_vertShaderModule != VK_NULL_HANDLE)
		vkDestroyShaderModule(m_device, m_vertShaderModule, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Create the shader module from SpirV code
//
VkShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}


//--------------------------------------------------------------------------------------------------
// Populate the shader stages information
//
void VulkanPipeline::populateShaderStages() {
	m_shaderStages = {};

	if (m_vulkanPipelineCreateInfo.vertShaderCode.size() > 0) {
		m_vertShaderModule = createShaderModule(m_vulkanPipelineCreateInfo.vertShaderCode);
		m_vertShaderStageInfo = {};
		m_vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		m_vertShaderStageInfo.module = m_vertShaderModule;
		m_vertShaderStageInfo.pName = "main";
		m_shaderStages.push_back(m_vertShaderStageInfo);
	}
	else {
		m_vertShaderModule = VK_NULL_HANDLE;
	}
	
	if (m_vulkanPipelineCreateInfo.fragShaderCode.size() > 0) {
		m_fragShaderModule = createShaderModule(m_vulkanPipelineCreateInfo.fragShaderCode);
		m_fragShaderStageInfo = {};
		m_fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		m_fragShaderStageInfo.module = m_fragShaderModule;
		m_fragShaderStageInfo.pName = "main";
		m_shaderStages.push_back(m_fragShaderStageInfo);
	}
	else {
		m_fragShaderModule = VK_NULL_HANDLE;
	}
}

//--------------------------------------------------------------------------------------------------
// Populate the vertex input stages information
//
void VulkanPipeline::populateVertexInput() {
	m_vertexInputInfo = {};
	m_vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vertexInputInfo.vertexBindingDescriptionCount = 1;
	m_vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vulkanPipelineCreateInfo.attributeDescriptions.size());
	m_vertexInputInfo.pVertexBindingDescriptions = &m_vulkanPipelineCreateInfo.bindingDescription;
	m_vertexInputInfo.pVertexAttributeDescriptions = m_vulkanPipelineCreateInfo.attributeDescriptions.data();

	m_inputAssemblyInfo = {};
	m_inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	m_inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
}

//--------------------------------------------------------------------------------------------------
// Populate the viewpoint information
//
void VulkanPipeline::populateViewPoint() {
	m_viewport = {};
	m_viewport.x = 0.0f;
	m_viewport.y = 0.0f;
	m_viewport.width = (float)m_vulkanPipelineCreateInfo.extent.width;
	m_viewport.height = (float)m_vulkanPipelineCreateInfo.extent.height;
	m_viewport.minDepth = 0.0f;
	m_viewport.maxDepth = 1.0f;

	m_scissor = {};
	m_scissor.offset = { 0, 0 };
	m_scissor.extent = m_vulkanPipelineCreateInfo.extent;

	m_viewportStateInfo = {};
	m_viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_viewportStateInfo.viewportCount = 1;
	m_viewportStateInfo.pViewports = &m_viewport;
	m_viewportStateInfo.scissorCount = 1;
	m_viewportStateInfo.pScissors = &m_scissor;
}

//--------------------------------------------------------------------------------------------------
// Populate the rasterization information
//
void VulkanPipeline::populateRasterization() {
	m_rasterizationInfo = {};
	m_rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_rasterizationInfo.depthClampEnable = VK_FALSE;
	m_rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	m_rasterizationInfo.lineWidth = 1.0f;
	m_rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	m_rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	m_rasterizationInfo.depthBiasEnable = VK_FALSE;
	m_rasterizationInfo.depthBiasConstantFactor = 0.0f;
	m_rasterizationInfo.depthBiasClamp = 0.0f;
	m_rasterizationInfo.depthBiasSlopeFactor = 0.0f;
}

//--------------------------------------------------------------------------------------------------
// Populate the multisampling information
//
void VulkanPipeline::populateMultisampling() {
	m_multisamplingInfo = {};
	m_multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_multisamplingInfo.sampleShadingEnable = VK_TRUE;
	m_multisamplingInfo.rasterizationSamples = m_vulkanPipelineCreateInfo.msaaSamples;
	m_multisamplingInfo.minSampleShading = .2f; // Optional
	m_multisamplingInfo.pSampleMask = nullptr; // Optional
	m_multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
	m_multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional
}

//--------------------------------------------------------------------------------------------------
// Populate the depth stencil information
//
void VulkanPipeline::populateDepthStencil() {
	m_depthStencilInfo = {};
	m_depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_depthStencilInfo.depthTestEnable = VK_TRUE;
	m_depthStencilInfo.depthWriteEnable = VK_TRUE;
	m_depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	m_depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	m_depthStencilInfo.minDepthBounds = 0.0f;
	m_depthStencilInfo.maxDepthBounds = 1.0f;
	m_depthStencilInfo.stencilTestEnable = VK_FALSE;
	m_depthStencilInfo.front = {};
	m_depthStencilInfo.back = {};
}

//--------------------------------------------------------------------------------------------------
// Populate the color blend information
//
void VulkanPipeline::populateColorBlend() {
	m_colorBlendAttachment = {};
	m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_colorBlendAttachment.blendEnable = VK_FALSE;

	m_colorBlendingInfo = {};
	m_colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_colorBlendingInfo.logicOpEnable = VK_FALSE;
	m_colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
	m_colorBlendingInfo.attachmentCount = 1;
	m_colorBlendingInfo.pAttachments = &m_colorBlendAttachment;
	m_colorBlendingInfo.blendConstants[0] = 0.0f;
	m_colorBlendingInfo.blendConstants[1] = 0.0f;
	m_colorBlendingInfo.blendConstants[2] = 0.0f;
	m_colorBlendingInfo.blendConstants[3] = 0.0f; 
}


}