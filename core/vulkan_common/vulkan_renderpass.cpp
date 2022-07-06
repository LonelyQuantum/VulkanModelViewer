#include "vulkan_renderpass.h"

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <array>
#include <vector>
namespace vkimpl {

/**
* The implementation of class VulkanRenderpassCreateInfo
*/

//Initialize the default values
const VkAttachmentDescription VulkanRenderPassCreateInfo::defaultColorAttachment = {
		{},
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
};
const VkAttachmentDescription VulkanRenderPassCreateInfo::defaultDepthAttachment = {
		{},
		VK_FORMAT_D32_SFLOAT,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
};
const VkAttachmentDescription VulkanRenderPassCreateInfo::defaultResolveAttachment = {
		{},
		VK_FORMAT_B8G8R8A8_SRGB,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
};

const VkSubpassDependency VulkanRenderPassCreateInfo::defaultDependency = {
	VK_SUBPASS_EXTERNAL,
	0,
	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	0,
	VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
};


//Constructor
VulkanRenderPassCreateInfo::VulkanRenderPassCreateInfo(bool hasColorInput, bool hasDepthInput, bool hasReolveInput) {
	hasColor = hasColorInput;
	hasDepth = hasDepthInput;
	hasResolve = hasReolveInput;

	colorAttachment = defaultColorAttachment;
	depthAttachment = defaultDepthAttachment;
	colorAttachmentResolve = defaultResolveAttachment;

	dependency = defaultDependency;
}

/**
* The implementation of class VulkanRenderpass
*/

//--------------------------------------------------------------------------------------------------
// Create the rasterization renderpass according to the create info
//
VkRenderPass VulkanRenderPass::createRenderPass(VulkanRenderPassCreateInfo info) {
	m_hasColor = info.hasColor;
	m_hasDepth = info.hasDepth;
	m_hasResolve = info.hasResolve;

	m_colorAttachment = info.colorAttachment;
	m_depthAttachment = info.depthAttachment;
	m_colorAttachmentResolve = info.colorAttachmentResolve;

	m_dependency = info.dependency;

	VkRenderPass renderPass{};

	std::vector<VkAttachmentDescription> attachments;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	//Populate and add color attachment
	if (m_hasColor) {
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = attachments.size();
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		attachments.push_back(m_colorAttachment);
	}
	if (m_hasDepth) {
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = attachments.size();
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		attachments.push_back(m_depthAttachment);
	}
	if (m_hasResolve) {
		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = attachments.size();
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
		attachments.push_back(m_colorAttachmentResolve);
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &m_dependency;

	if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	return renderPass;
}
}