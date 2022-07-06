#ifndef VULKAN_RENDERPASS
#define VULKAN_RENDERPASS

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"
#include <string>

namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::VulkanRenderPassCreateInfo
vkimpl::VulkanRenderPassCreateInfo contains the information we need to initialize the Vulkan renderpass
*/
struct VulkanRenderPassCreateInfo {
	VulkanRenderPassCreateInfo(bool hasColorInput = false, bool hasDepthInput = false, bool hasReolveInput = false);

	bool hasColor;
	bool hasDepth;
	bool hasResolve;

	VkAttachmentDescription colorAttachment;
	VkAttachmentDescription depthAttachment;
	VkAttachmentDescription colorAttachmentResolve;

	VkSubpassDependency dependency;

private:
	static const VkAttachmentDescription defaultColorAttachment;
	static const VkAttachmentDescription defaultDepthAttachment;
	static const VkAttachmentDescription defaultResolveAttachment;
	static const VkSubpassDependency defaultDependency;
};

/**
\class vkimpl::VulkanRenderpass
vkimpl::VulkanRenderpass handles the creation Vulkan renderpass
*/
class VulkanRenderPass {
public:
	VulkanRenderPass(VkDevice device = VK_NULL_HANDLE)
		: m_device(device) {};
	VkRenderPass createRenderPass(VulkanRenderPassCreateInfo info);

	VkDevice m_device;

private:
	bool m_hasColor;
	bool m_hasDepth;
	bool m_hasResolve;

	VkAttachmentDescription m_colorAttachment;
	VkAttachmentDescription m_depthAttachment;
	VkAttachmentDescription m_colorAttachmentResolve;

	VkSubpassDependency m_dependency;
};
}
#endif // !VULKAN_RENDERPASS