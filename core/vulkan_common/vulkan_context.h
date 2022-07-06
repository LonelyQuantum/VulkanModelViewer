#ifndef VULKAN_CONTEXT
#define VULKAN_CONTEXT

#include "vulkan_common.h"

#include "vulkan_common_utils.h"

#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <optional>
#include <string>
#include <set>
#include <map>

namespace vkimpl 
{
/**
* Containers and helpers of vulkan API
*/

/**
\struct vkimpl::VulkanContextCreateInfo
vkimpl::VulkanContextCreateInfo contains the information we need to initialize the Vulkan context, 
including instance, physical device and logical device
*/
struct VulkanContextCreateInfo {
	uint32_t apiMajor{ 1 };
	uint32_t apiMinor{ 2 };
	std::vector<const char*> instanceLayers{};
	std::vector<const char*> instanceExtensions{};
	std::vector<void* > EXTPhysicalDeviceFeatureStructs{};
	std::map<VkStructureType, std::vector<const char*>> physicalDeviceFeatureRequirements{};
	std::vector<const char*> deviceExtensions{};
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	//Setters
	void setApiVersion(uint32_t apiMajorInput = 1, uint32_t apiMinorInput = 2);
	void addInstanceLayer(const char* layer);
	void addInstanceExtension(const char* extension);
	void addDeviceExtension(const char* extension, void* pPhysicalDeviceFeatureStruct = VK_NULL_HANDLE, std::vector<const char*> featureRequirements = {});
	void addPhysicalDeviceFeatureRequirement(VkStructureType featureStructType, const char* feature);

	std::string toString();
};


/**
\class vkimpl::VulkanContext
vkimpl::VulkanContext handles the creation of lower level Vulkan instances such as VkInstance,
VkPhysicalDevice and VkDevicewhich depends on the environment, and is the same for different applications.
*/
class VulkanContext {
public:
	void init(VulkanContextCreateInfo info = {});
	void createInstance(VulkanContextCreateInfo info = {});
	void createPhysicalDevice(VulkanContextCreateInfo info = {}, VkSurfaceKHR surface = VK_NULL_HANDLE);
	void createDevice(VulkanContextCreateInfo info = {});

	//Public helpers
	std::vector<VkPhysicalDevice> getCompatiblePhysicalDevices(VulkanContextCreateInfo info = VulkanContextCreateInfo{}, VkSurfaceKHR surface = VK_NULL_HANDLE);

	//Public members
	uint32_t			m_apiMajor{ 1 };
	uint32_t			m_apiMinor{ 2 };
	VkInstance			m_instance{ VK_NULL_HANDLE };
	VkPhysicalDevice	m_physicalDevice{ VK_NULL_HANDLE };
	VkDevice			m_device{ VK_NULL_HANDLE };

	//Physical Device Features
	VkPhysicalDeviceFeatures2 m_physicalFeaturesStructChain{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	VkPhysicalDeviceFeatures         m_features10{};
	VkPhysicalDeviceVulkan11Features m_features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
	VkPhysicalDeviceVulkan12Features m_features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	
	bool m_enableValidationLayers;
	std::vector<const char*> m_instanceLayers{};
	std::vector<const char*> m_instanceExtensions{};
	std::vector<void*> m_EXTPhysicalDeviceFeatureStructs{};
	std::vector<const char*> m_deviceExtensions{};

	QueueFamilyIndices m_queueFamilyIndices;
	VkQueue m_graphicsQueue;
	VkQueue m_computeQueue;
	VkQueue m_transferQueue;
	VkQueue m_presentQueue;

	VkDebugUtilsMessengerEXT m_debugMessenger;

private:
	bool checkValidationLayerSupport();

	bool isDeviceSuitable(VkPhysicalDevice device, VulkanContextCreateInfo info, VkSurfaceKHR surface = VK_NULL_HANDLE);
	QueueFamilyIndices  findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions);
	void constructStructChains(VulkanContextCreateInfo info);
	bool checkDeviceFeaturesSupport(VkPhysicalDevice device, std::map<VkStructureType, std::vector<const char*>> physicalDeviceFeatureRequirements);
	bool checkDeviceFeaturesSupport(VkPhysicalDevice device, void* pFeatureStructCast, std::vector<const char*> requiredFeatures);

	VulkanCommonUtils _utils;
};
}
#endif // !VULKAN_CONTEXT