#ifndef VULKAN_SWAPCHAIN
#define VULKAN_SWAPCHAIN

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"

#include <vector>
namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::VulkanSwapchainCreateInfo
vkimpl::VulkanSwapchainCreateInfo contains the information we need to initialize the Vulkan swapchain
*/
struct VulkanSwapchainCreateInfo {

};

/**
\struct vkimpl::SwapChainSupportDetails the swapchain supports of the current physical device
*/
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

/**
\class vkimpl::VulkanSwapchain
vkimpl::VulkanSwapchain handles the creation Vulkan swapchain including swapchain images and imageviews,
and the corresponding buffers, semaphores and barriers
*/
class VulkanSwapchain {
public:
	VulkanSwapchain() = default;
	VulkanSwapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, QueueFamilyIndices indices)
		:m_physicalDevice(physicalDevice), m_surface(surface), m_device(device), m_indices(indices){};
	void init(int width, int height);
	void init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, QueueFamilyIndices indices, int width, int height);
	void createSwapchain(int width, int height);
	void createImageViews();

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkSurfaceKHR m_surface;
	QueueFamilyIndices m_indices;

	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchainImages;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;
	std::vector<VkImageView> m_swapchainImageViews;

private:
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height);
};
}
#endif // !VULKAN_SWAPCHAIN