#ifndef VULKAN_PIPELINES
#define VULKAN_PIPELINES

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"

#include <vector>

namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::VulkanRenderPassCreateInfo
vkimpl::VulkanRenderPassCreateInfo contains the information we need to initialize the Vulkan renderpass
*/
struct VulkanPipelineCreateInfo {
	std::vector<char> vertShaderCode;
	std::vector<char> fragShaderCode;

	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkExtent2D extent;
	VkSampleCountFlagBits msaaSamples;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	VkRenderPass renderPass;
};

/**
\class vkimpl::VulkanPipeline
vkimpl::VulkanPipeline handles the creation of Vulkan pipelines
*/
class VulkanPipeline {
public:
	VulkanPipeline(VkDevice device = VK_NULL_HANDLE)
		: m_device(device) { };	

	void initAndCreateGraphicsPipeline(VulkanPipelineCreateInfo info, VkPipelineLayout& graphicsPipelineLayout, VkPipeline& pipeline);
	void initGraphicsPipelineCreateInfo(VulkanPipelineCreateInfo info);
	void createGraphicsPipelineLayout(VkPipelineLayout& graphicsPipelineLayout);
	void createGraphicsPipeline(VkPipeline& pipeline);

	VkShaderModule VulkanPipeline::createShaderModule(const std::vector<char>& code);

	VkDevice m_device;
	VulkanPipelineCreateInfo m_vulkanPipelineCreateInfo;

	VkShaderModule m_vertShaderModule;
	VkShaderModule m_fragShaderModule;
	VkPipelineShaderStageCreateInfo m_vertShaderStageInfo;
	VkPipelineShaderStageCreateInfo m_fragShaderStageInfo;
	std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;

	VkPipelineVertexInputStateCreateInfo m_vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo;

	VkViewport m_viewport;
	VkRect2D m_scissor;
	VkPipelineViewportStateCreateInfo m_viewportStateInfo;

	VkPipelineRasterizationStateCreateInfo m_rasterizationInfo;

	VkPipelineMultisampleStateCreateInfo m_multisamplingInfo;

	VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo;

	VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo m_colorBlendingInfo;

	VkPipelineLayoutCreateInfo m_graphicsPipelineLayoutInfo;
	VkPipelineLayout m_graphicsPipelineLayout;

	VkGraphicsPipelineCreateInfo m_graphicsPipelineCreateInfo;
	VkPipeline m_graphicsPipeline;
	

private:
	void populateShaderStages();
	void populateVertexInput();
	void populateViewPoint();
	void populateRasterization();
	void populateMultisampling();
	void populateDepthStencil();
	void populateColorBlend();
};
}
#endif // !VULKAN_PIPELINES