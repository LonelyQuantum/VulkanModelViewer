#ifndef VULKAN_COMMANDS
#define VULKAN_COMMANDS

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"

#include <vector>

namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::VulkanCommandsCreateInfo
vkimpl::VulkanCommandsCreateInfo contains the information we need to initialize the Vulkan command pools and command buffers
*/
struct VulkanCommandsCreateInfo {

};

/**
\class vkimpl::VulkanCommands
vkimpl::VulkanCommands handles the creation Vulkan command pools and command buffers
*/
class VulkanCommands {
public:
	VulkanCommands() = default;
	VulkanCommands(VkDevice device, VkCommandPool commandPool = VK_NULL_HANDLE, uint32_t queueFamilyIndex = 0)
		: m_device(device), m_commandPool(commandPool), m_queueFamilyIndex(queueFamilyIndex) { };

	void init(uint32_t queueFamilyIndex, int commandBufferSize, VkCommandPoolCreateFlags flags = 0);
	void setPoolInfo(VkCommandPool commandPool, uint32_t queueFamilyIndex = 0);
	VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	std::vector<VkCommandBuffer> createCommandBuffers(VkCommandPool commandPool, int size);

	VkCommandBuffer beginSingleTimeCommands();
	void VulkanCommands::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);

	uint32_t m_queueFamilyIndex{ 0 };
	VkDevice m_device{ VK_NULL_HANDLE };
	int m_commandBufferSize{ 0 };

	VkCommandPool m_commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> m_commandBuffers;

private:
	
};
}
#endif // !VULKAN_COMMANDS