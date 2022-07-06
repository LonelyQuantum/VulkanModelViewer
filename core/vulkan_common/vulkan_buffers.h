#ifndef VULKAN_BUFFERS
#define VULKAN_BUFFERS

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"

#include <vector>

namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\class vkimpl::VulkanBuffers
vkimpl::VulkanBuffers handles the creation of Vulkan buffers and data tranfer involving buffers
*/
class VulkanBuffers {
public:
	VulkanBuffers(VkPhysicalDevice physicalDevice = VK_NULL_HANDLE, VkDevice device = VK_NULL_HANDLE, VkCommandPool commandPool = VK_NULL_HANDLE, VkQueue queue = VK_NULL_HANDLE)
		: m_physicalDevice(physicalDevice), m_device(device), m_commandPool(commandPool), m_queue(queue) { };	

	void fillBufferData(VkBuffer& buffer, void* bufferData, VkDeviceSize bufferSize);

	void VulkanBuffers::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void VulkanBuffers::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageAspectFlags aspectMask);
	void VulkanBuffers::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkCommandPool m_commandPool;
	VkQueue m_queue;

private:
	
};
}
#endif // !VULKAN_BUFFERS