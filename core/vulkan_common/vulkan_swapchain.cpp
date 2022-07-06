#include "vulkan_swapchain.h"

#include "vulkan_images.h"

#include <algorithm>
#include <stdexcept>
namespace vkimpl {

/**
* The implementation of class VulkanSwapchain
*/

//--------------------------------------------------------------------------------------------------
// Create the Vulkan swapchain and swapchain imageviews
//
void VulkanSwapchain::init(int width, int height) {
	createSwapchain(width, height);
	createImageViews();
}

//--------------------------------------------------------------------------------------------------
// Initialize the Vulkan swapchain and swapchain imageviews
//
void VulkanSwapchain::init(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice device, QueueFamilyIndices indices, int width, int height) {
	m_physicalDevice = physicalDevice;
	m_surface = surface;
	m_device = device;
	m_indices = indices;

	createSwapchain(width, height);
	createImageViews();
}

//--------------------------------------------------------------------------------------------------
// Create the swapchain and its swapchain images
//
void VulkanSwapchain::createSwapchain(int width, int height) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { m_indices.graphicsFamily.value(), m_indices.presentFamily.value() };

	if (m_indices.graphicsFamily != m_indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

//--------------------------------------------------------------------------------------------------
// Create the image view for SceneEditor
//
void VulkanSwapchain::createImageViews() {
	VulkanImages imageHelper(m_physicalDevice, m_device);
	m_swapchainImageViews.resize(m_swapchainImages.size());

	for (uint32_t i = 0; i < m_swapchainImages.size(); i++) {
		VulkanImageInfo swapchainImageViewInfo{};
		swapchainImageViewInfo.format = m_swapchainImageFormat;
		swapchainImageViewInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		imageHelper.setImageInfo(swapchainImageViewInfo);
		m_swapchainImageViews[i] = imageHelper.createImageView(m_swapchainImages[i]);
	}
}

//--------------------------------------------------------------------------------------------------
// Return the supported swapchain types of the physical device and the surface
//
SwapChainSupportDetails VulkanSwapchain::querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

//--------------------------------------------------------------------------------------------------
// Specify the surface format for the currrent SwapChain
//
VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

//--------------------------------------------------------------------------------------------------
// Specify the present mode format for the currrent SwapChain, prefer VK_PRESENT_MODE_MAILBOX_KHR
//
VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

//--------------------------------------------------------------------------------------------------
// Specify the extent for the currrent Swapchain given the current window shape
//
VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

}