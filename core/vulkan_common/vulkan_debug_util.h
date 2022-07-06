#ifndef VULKAN_DEBUG_UTIL
#define VULKAN_DEBUG_UTIL
#include <vulkan/vulkan_core.h>

#include "tools.h" 
#include "vulkan_common_utils.h"

#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>
#include <string>
#include <set>

namespace vkimpl 
{
/**
* Containers and helpers of vulkan API
*/

/**
* \class vkimpl::VulkanDebugUtil
* vkimpl::VulkanDebugUtil contains the debug helpers that enriches the functions of validation layers
*/
class VulkanDebugUtil 
{
public:
	VulkanDebugUtil() = default;
	VulkanDebugUtil(VkInstance instance, VkDevice device)
		: m_instance(instance), m_device(device) { }

	VkInstance m_instance;
	VkDevice m_device;
	VulkanCommonUtils m_utils;

	static const VkDebugUtilsMessageSeverityFlagsEXT severity = 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	int colNum{ 3 };

	//Release the resources
	void destroy();

	//Print structs
	void printVkPhysicalDeviceFeatures(VkPhysicalDeviceFeatures features, int colNum = 4);
	void printVkPhysicalDeviceFeatures(void* features, int colNum = 3);
	void printVkPhysicalDeviceFeatureStructChain(void* structFeatures, int colNum = 3);

	//Set debug object name
	void setObjectName(const uint64_t object, const std::string& name, VkObjectType t);
	void setObjectName(VkBuffer object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER); }
	void setObjectName(VkBufferView object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_BUFFER_VIEW); }
	void setObjectName(VkCommandBuffer object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_BUFFER); }
	void setObjectName(VkCommandPool object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_COMMAND_POOL); }
	void setObjectName(VkDescriptorPool object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_POOL); }
	void setObjectName(VkDescriptorSet object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET); }
	void setObjectName(VkDescriptorSetLayout object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT); }
	void setObjectName(VkDevice object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE); }
	void setObjectName(VkDeviceMemory object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_DEVICE_MEMORY); }
	void setObjectName(VkFramebuffer object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_FRAMEBUFFER); }
	void setObjectName(VkImage object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE); }
	void setObjectName(VkImageView object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_IMAGE_VIEW); }
	void setObjectName(VkPipeline object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE); }
	void setObjectName(VkPipelineLayout object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_PIPELINE_LAYOUT); }
	void setObjectName(VkQueryPool object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_QUERY_POOL); }
	void setObjectName(VkQueue object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_QUEUE); }
	void setObjectName(VkRenderPass object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_RENDER_PASS); }
	void setObjectName(VkSampler object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SAMPLER); }
	void setObjectName(VkSemaphore object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SEMAPHORE); }
	void setObjectName(VkShaderModule object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SHADER_MODULE); }
	void setObjectName(VkSwapchainKHR object, const std::string& name) { setObjectName((uint64_t)object, name, VK_OBJECT_TYPE_SWAPCHAIN_KHR); }
	
	//Implementations
	VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo);


	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	
	//Static helpers
	static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
private:
	int alignmentDefault = 60;
	std::map<std::string, int> valuePrintColNum{
		{ "VkPhysicalDeviceFeatures2", 4 }, { "VkPhysicalDeviceVulkan11Features", 4 }, { "VkPhysicalDeviceVulkan12Features", 3 },
		{ "VkPhysicalDeviceAccelerationStructureFeaturesKHR", 3 }
	};

	std::map<std::string, int> valuePrintAlignment{
		{ "VkPhysicalDeviceFeatures2", 45 }, { "VkPhysicalDeviceVulkan11Features", 45 }, { "VkPhysicalDeviceVulkan12Features", 60 },
		{ "VkPhysicalDeviceAccelerationStructureFeaturesKHR", 45 }
	};
	
};
}
#endif