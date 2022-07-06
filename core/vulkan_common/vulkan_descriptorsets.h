#ifndef VULKAN_DESCRIPTORSETS
#define VULKAN_DESCRIPTORSETS

#include <vulkan/vulkan_core.h>

#include "vulkan_common.h"
#include <string>
#include <vector>

namespace vkimpl
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::DescriptorSetLayoutBindingInfo
vkimpl::DescriptorSetLayoutBindingInfo contains the binding information for descriptor set layout
*/
struct DescriptorSetLayoutBindingInfo {
	VkDescriptorType descriptorType;
	uint32_t descriptorCount;
	VkShaderStageFlags stageFlags;
};

/**
\struct vkimpl::DescriptorSetInfo
vkimpl::DescriptorSetInfo contains layout bindings and resources to create certain descriptor set
*/
struct DescriptorSetInfo {
	std::vector<DescriptorSetLayoutBindingInfo> bindingInfos;
	std::vector<VkDescriptorBufferInfo> bufferInfos;
	std::vector<VkDescriptorImageInfo> imageInfos;
};

/**
\class vkimpl::VulkanDescriptorSets
vkimpl::VulkanDescriptorSets handles the creation Vulkan descritpor set layout, descriptor pool and descriptor sets
*/
class VulkanDescriptorSets {
public:
	VulkanDescriptorSets(VkDevice device = VK_NULL_HANDLE)
		:m_device(device) { };
	void createDescriptorSetLayout(std::vector<DescriptorSetLayoutBindingInfo> bindingInfos, VkDescriptorSetLayout& descriptorSetLayout);
	void createDescriptorPool(int maxSets, std::vector<VkDescriptorPoolSize> poolSizes, VkDescriptorPool& descriptorPool);
	void createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, DescriptorSetInfo descriptorSetInfo, VkDescriptorSet& descriptorSet);
	void createDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<DescriptorSetInfo> descriptorSetInfos, std::vector<VkDescriptorSet>& descriptorSets);

	VkDevice m_device;

private:
};
}
#endif // !VULKAN_DESCRIPTORSETS