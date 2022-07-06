#include "vulkan_debug_util.h"
#include <iomanip>

namespace vkimpl {

//--------------------------------------------------------------------------------------------------
// Helper function to print VkPhysicalDeviceFeatures struct
//
void VulkanDebugUtil::printVkPhysicalDeviceFeatures(VkPhysicalDeviceFeatures features, int colNum) {
	std::vector<VkBool32>  featureValues = m_utils.getVkBool32StructValues(features);
	std::cout << "\n=============================Print VkPhysicalDeviceFeatures=============================";
	for (int i = 0; i < featureValues.size(); i++) {
		if (i % colNum == 0) std::cout << "\n";
		std::cout << std::setw(45) << std::left << std::string(m_utils.physicalDeviceFeatures2Vector[i]) + ": " + std::to_string(featureValues[i]);
	}
	std::cout << std::endl;
}

//--------------------------------------------------------------------------------------------------
// Helper function to print VkPhysicalDeviceFeatures struct
//
void VulkanDebugUtil::printVkPhysicalDeviceFeatures(void* features, int colNumInput) {
	std::string featureStructName = m_utils.getVkBool32StructName(features);
	std::vector<const char*> featureStructMemberNames = m_utils.getVkBool32StructVector(features);
	if (!featureStructMemberNames.empty()) {
		std::vector<VkBool32>  featureValues = m_utils.getVkBool32StructValues(features);
		std::cout << "\n=============================Print " + featureStructName + "============================ = ";
		int colNum = colNumInput;
		if (valuePrintColNum.find(featureStructName) != valuePrintColNum.end()) {
			colNum = valuePrintColNum[featureStructName];
		}
		int alignment = alignmentDefault;
		if (valuePrintAlignment.find(featureStructName) != valuePrintAlignment.end()) {
			alignment = valuePrintAlignment[featureStructName];
		}
		for (int i = 0; i < featureValues.size(); i++) {
			if (i % colNum == 0) std::cout << "\n";
			std::cout << std::setw(alignment) << std::left << std::string(featureStructMemberNames[i]) + ": " + std::to_string(featureValues[i]);
		}
		std::cout << std::endl;
	}
	else std::cout << "\nPrint not support for this feature structure: " << featureStructName << std::endl;
}

//--------------------------------------------------------------------------------------------------
// Helper function to print VkPhysicalDeviceFeatures struct
//
void VulkanDebugUtil::printVkPhysicalDeviceFeatureStructChain(void* structFeatures, int colNum) {
	VulkanStructCommon* features = reinterpret_cast<VulkanStructCommon*>(structFeatures);
	while (features != nullptr) {
		printVkPhysicalDeviceFeatures(features);
		features = (VulkanStructCommon*)features->pNext;
	}
}

//--------------------------------------------------------------------------------------------------
// Set the name of a Vulkan object in validation layer
//
void VulkanDebugUtil::setObjectName(const uint64_t object, const std::string& name, VkObjectType t)
{
	VkDebugUtilsObjectNameInfoEXT s{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr, t, object, name.c_str() };
	vkSetDebugUtilsObjectNameEXT(m_device, &s);
}

//--------------------------------------------------------------------------------------------------
// Implementation of vkSetDebugUtilsObjectNameEXT API
//
VKAPI_ATTR VkResult VKAPI_CALL VulkanDebugUtil::vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo)
{
	PFN_vkSetDebugUtilsObjectNameEXT pfn_vkSetDebugUtilsObjectNameEXT = 0;
	pfn_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT");
	return pfn_vkSetDebugUtilsObjectNameEXT(device, pNameInfo);
}

/**
* Static functions
*/
//--------------------------------------------------------------------------------------------------
// Prepare VkDebugUtilsMessengerCreateInfoEXT for VkDebugUtilsMessengerEXT, assign the severity levels
//
void VulkanDebugUtil::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

//--------------------------------------------------------------------------------------------------
// The debug callback function used to print validation layer messages
//
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugUtil::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

//--------------------------------------------------------------------------------------------------
// Create the VkDebugUtilsMessengerEXT
//
VkResult VulkanDebugUtil::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//--------------------------------------------------------------------------------------------------
// Destroy the VkDebugUtilsMessengerEXT
//
void VulkanDebugUtil::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
};
}