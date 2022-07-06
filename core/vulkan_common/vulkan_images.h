#ifndef VULKAN_IMAGES
#define VULKAN_IMAGES

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"

#include <vector>

namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::VulkanImageInfo
vkimpl::VulkanImageInfo contains the information of the image and image view under operation
*/
struct VulkanImageInfo {
	//Image info
	VkFormat format;
	VkExtent3D extent;
	VkSampleCountFlagBits numSamples;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	uint32_t mipLevels{ 1 };

	//Memory info
	VkMemoryPropertyFlags properties;

	//Image view info
	VkImageAspectFlags aspectFlags;

	std::string toString();
};

/**
\class vkimpl::VulkanImages
vkimpl::VulkanImages handles the creation Vulkan images and imageviews, and the layout transition of them
*/
class VulkanImages {
public:
	VulkanImages(VkPhysicalDevice physicalDevice = VK_NULL_HANDLE, VkDevice device = VK_NULL_HANDLE, VkCommandPool commandPool = VK_NULL_HANDLE, VkQueue queue = VK_NULL_HANDLE, VulkanImageInfo info = {})
		: m_physicalDevice(physicalDevice), m_device(device), m_commandPool(commandPool), m_queue(queue), m_currentImageInfo(info) { };
	
	void setOperationInfo(VkCommandPool commandPool = VK_NULL_HANDLE, VkQueue queue = VK_NULL_HANDLE, VulkanImageInfo info = {});

	void setImageInfo(VulkanImageInfo imageInfo);

	void createImage(VkImage& image, VkDeviceMemory& imageMemory);
	void fillImagePixels(VkImage& image, void* pixels, VkDeviceSize imageSize, VkImageLayout originalLayout, VkImageAspectFlags aspectMask);
	VkImageView createImageView(VkImage image);
	
	void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	void generateMipmaps(VkImage image);

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkCommandPool m_commandPool;
	VkQueue m_queue;
	VulkanImageInfo m_currentImageInfo;

private:
	
};
}
#endif // !VULKAN_IMAGES