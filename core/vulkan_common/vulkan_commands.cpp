#include "vulkan_commands.h"

#include <stdexcept>
namespace vkimpl {

/**
* The implementation of class VulkanCommands
*/

//--------------------------------------------
// Initialize the Vulkan command pool and command buffers
//
void VulkanCommands::init(uint32_t queueFamilyIndex, int commandBufferSize, VkCommandPoolCreateFlags flags){
	m_queueFamilyIndex = queueFamilyIndex;
	m_commandBufferSize = commandBufferSize;
	createCommandPool(queueFamilyIndex, flags);
	createCommandBuffers(m_commandPool, m_commandBufferSize);
}

//--------------------------------------------
// Set command pool and related info
//
void VulkanCommands::setPoolInfo(VkCommandPool commandPool, uint32_t queueFamilyIndex) {
	m_commandPool = commandPool;
	m_queueFamilyIndex = queueFamilyIndex;
}

//--------------------------------------------
// Create a command pool on the graphics queue family of given device
//
VkCommandPool VulkanCommands::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = flags;

	if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics command pool!");
	}

	return m_commandPool;
}

//--------------------------------------------------------------------------------------------------
// Create the comman buffers for each swapchain image
//
std::vector<VkCommandBuffer> VulkanCommands::createCommandBuffers(VkCommandPool commandPool, int size) {
	m_commandBuffers.resize(size);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	return m_commandBuffers;
}


//--------------------------------------------------------------------------------------------------
// Create and begin recording a one-time command buffer and return it
//
VkCommandBuffer VulkanCommands::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

//--------------------------------------------------------------------------------------------------
// End recording and submit a one-time command butter
//
void VulkanCommands::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}
}