#include "vulkan_descriptorsets.h"

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <array>
#include <vector>

namespace vkimpl {

/**
* The implementation of class VulkanDescriptorSetsCreateInfo
*/


/**
* The implementation of class VulkanDescriptorSets
*/

//--------------------------------------------------------------------------------------------------
// Create the descriptor set layout
//
void VulkanDescriptorSets::createDescriptorSetLayout(std::vector<DescriptorSetLayoutBindingInfo> bindingInfos, VkDescriptorSetLayout& descriptorSetLayout) {
	uint32_t numBindings = bindingInfos.size();
	
	std::vector<VkDescriptorSetLayoutBinding> bindings(numBindings);

	uint32_t currentBinding = 0;
	for (int bindingIx = 0; bindingIx < numBindings; bindingIx++) {
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.descriptorCount = bindingInfos[bindingIx].descriptorCount;
		layoutBinding.descriptorType = bindingInfos[bindingIx].descriptorType;
		layoutBinding.binding = currentBinding;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = bindingInfos[bindingIx].stageFlags;
		bindings[bindingIx] = layoutBinding;
		currentBinding += layoutBinding.descriptorCount;
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());;
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

//--------------------------------------------------------------------------------------------------
// Create the descriptor pool given pool sizes and max sets
//
void VulkanDescriptorSets::createDescriptorPool(int maxSets, std::vector<VkDescriptorPoolSize> poolSizes, VkDescriptorPool& descriptorPool) {
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(maxSets);

	if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

//--------------------------------------------------------------------------------------------------
// Allocate the a single descriptor set from descriptor pool given its layout and resources
//
void VulkanDescriptorSets::createDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, DescriptorSetInfo descriptorSetInfo, VkDescriptorSet& descriptorSet) {
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
		std::cout << vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet) << std::endl;
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites{};
	uint32_t currentBinding = 0;
	uint32_t currentBuffer = 0;
	uint32_t currentImage = 0;
	for (int bindingIx = 0; bindingIx < descriptorSetInfo.bindingInfos.size(); bindingIx++) {
		VkDescriptorType descriptorType = descriptorSetInfo.bindingInfos[bindingIx].descriptorType;
		uint32_t descriptorCount = descriptorSetInfo.bindingInfos[bindingIx].descriptorCount;
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = currentBinding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = descriptorType;
		descriptorWrite.descriptorCount = descriptorCount;
		currentBinding += descriptorCount;
		if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			descriptorWrite.pBufferInfo = &descriptorSetInfo.bufferInfos[currentBuffer];
			currentBuffer += descriptorCount;
		}
		else if (descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
			descriptorWrite.pImageInfo = &descriptorSetInfo.imageInfos[currentImage];
			currentImage += descriptorCount;
		}
		descriptorWrites.push_back(descriptorWrite);
	}

	vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Allocate the descriptor sets from descriptor pool given layouts and resources
//
void VulkanDescriptorSets::createDescriptorSets(VkDescriptorPool descriptorPool, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<DescriptorSetInfo> descriptorSetInfos, std::vector<VkDescriptorSet>& descriptorSets) {
	uint32_t numDescriptorSets = static_cast<uint32_t>(descriptorSetLayouts.size());
	
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = numDescriptorSets;
	allocInfo.pSetLayouts = descriptorSetLayouts.data();

	descriptorSets.resize(numDescriptorSets);
	if (vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
		std::cout << vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets.data()) << std::endl;
	}

	for (uint32_t setIx = 0; setIx < numDescriptorSets; setIx++) {
		std::vector<VkWriteDescriptorSet> descriptorWrites{};
		uint32_t currentBinding = 0;
		uint32_t currentBuffer = 0;
		uint32_t currentImage = 0;
		for (int bindingIx = 0; bindingIx < descriptorSetInfos[setIx].bindingInfos.size(); bindingIx++) {
			VkDescriptorType descriptorType = descriptorSetInfos[setIx].bindingInfos[bindingIx].descriptorType;
			uint32_t descriptorCount = descriptorSetInfos[setIx].bindingInfos[bindingIx].descriptorCount;
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[setIx];
			descriptorWrite.dstBinding = currentBinding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = descriptorType;
			descriptorWrite.descriptorCount = descriptorCount;
			currentBinding += descriptorCount;
			if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
				descriptorWrite.pBufferInfo = &descriptorSetInfos[setIx].bufferInfos[currentBuffer];
				currentBuffer += descriptorCount;
			}
			else if (descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
				descriptorWrite.pImageInfo = &descriptorSetInfos[setIx].imageInfos[currentImage];
				currentImage += descriptorCount;
			}
			descriptorWrites.push_back(descriptorWrite);
		}

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

}