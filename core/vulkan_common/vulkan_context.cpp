#include "vulkan_context.h"

#include "tools.h" 
#include "vulkan_debug_util.h"
namespace vkimpl {
/** 
* The implementation of struct VulkanContextCreateInfo
*/
//--------------------------------------------------------------------------------------------------
// Set Vulkan API version
//
void VulkanContextCreateInfo::setApiVersion(uint32_t apiMajorInput, uint32_t apiMinorInput) {
	apiMajor = apiMajorInput;
	apiMinor = apiMinorInput;
}

//--------------------------------------------------------------------------------------------------
// Set Vulkan instance layers
//
void VulkanContextCreateInfo::addInstanceLayer(const char* layer) {
	add_unique(instanceLayers, layer);
}

//--------------------------------------------------------------------------------------------------
// Set Vulkan instance extension
//
void VulkanContextCreateInfo::addInstanceExtension(const char* extension) {
	add_unique(instanceExtensions, extension);
}

//--------------------------------------------------------------------------------------------------
// Set Vulkan device extensions
//
void VulkanContextCreateInfo::addDeviceExtension(const char* extension, void* pPhysicalDeviceFeatureStruct, std::vector<const char*> featureRequirements) {
	add_unique(deviceExtensions, extension);
	if (pPhysicalDeviceFeatureStruct != nullptr) {
		EXTPhysicalDeviceFeatureStructs.push_back(pPhysicalDeviceFeatureStruct);
		VkStructureType sType = reinterpret_cast<ExtensionHeader*>(pPhysicalDeviceFeatureStruct)->sType;
		if (physicalDeviceFeatureRequirements.find(sType) != physicalDeviceFeatureRequirements.end())
			physicalDeviceFeatureRequirements[sType].insert(physicalDeviceFeatureRequirements[sType].end(), featureRequirements.begin(), featureRequirements.end());
		else if (featureRequirements.size() > 0) physicalDeviceFeatureRequirements[sType] = featureRequirements;
	}
}

//--------------------------------------------------------------------------------------------------
// Set Vulkan physical device feature requriements
//
void VulkanContextCreateInfo::addPhysicalDeviceFeatureRequirement(VkStructureType featureStructType, const char* feature) {
	if (physicalDeviceFeatureRequirements.find(featureStructType) != physicalDeviceFeatureRequirements.end())
		add_unique(physicalDeviceFeatureRequirements[featureStructType], feature);
	else
		physicalDeviceFeatureRequirements[featureStructType] = std::vector<const char*>{ feature };
}

//--------------------------------------------------------------------------------------------------
// Convert the vulakn context create information to string
//
std::string VulkanContextCreateInfo::toString() {
	std::string str;
	str += "API Version: " + std::to_string(apiMajor) + "." + std::to_string(apiMinor);
	str += "\nInstance Layers:\n";
	for (const char* layer : instanceLayers) { str += std::string(layer); str += ", "; }
	if (!instanceLayers.empty()) str.erase(str.length() - 2);
	str += "\nInstance Extensions:\n";
	for (const char* extension : instanceExtensions) { str += extension; str += ", "; }
	if (!instanceExtensions.empty()) str.erase(str.length() - 2);
	str += "\nDevice Extensions:\n";
	for (const char* extension : deviceExtensions) { str += extension; str += ", "; }
	if (!deviceExtensions.empty()) str.erase(str.length() - 2);
	str +=  "\nDebugUtilStype: " + std::to_string(debugCreateInfo.sType);
	return str;
}

/** 
* The implementation of struct VulkanContext
*/
//--------------------------------------------------------------------------------------------------
// Initailize the Vulkan Context with no present support
//
void VulkanContext::init(VulkanContextCreateInfo info) {
	//Create the vulkan instance
	createInstance(info);
	
	//Select an appropriate physical device
	createPhysicalDevice(info);
	createDevice(info);
}

//--------------------------------------------------------------------------------------------------
// Create the Vulkan instance that has given layers, and setup debug messenger if validation layers are enabled
//
void VulkanContext::createInstance(VulkanContextCreateInfo info) {
	//Set up vulkan instance
	m_apiMajor = info.apiMajor;
	m_apiMinor = info.apiMinor;
	m_instanceLayers = info.instanceLayers;
	m_enableValidationLayers = !m_instanceLayers.empty();
	m_instanceExtensions = info.instanceExtensions;
	m_EXTPhysicalDeviceFeatureStructs = info.EXTPhysicalDeviceFeatureStructs;
	m_deviceExtensions = info.deviceExtensions;

	//Check the availability for required validation layers
	if (m_enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "MyApp";
	appInfo.pEngineName = "No Engine";
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, m_apiMajor, m_apiMinor, 0);;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = m_instanceExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (m_enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_instanceLayers.size());
		createInfo.ppEnabledLayerNames = m_instanceLayers.data();
		//Enable validation layer during instance creation and destruction
		if (!info.debugCreateInfo.sType) VulkanDebugUtil::populateDebugMessengerCreateInfo(debugCreateInfo);
		else debugCreateInfo = info.debugCreateInfo;
		
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	//Setup debug messenger
	if (m_enableValidationLayers) {
		if (VulkanDebugUtil::CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Create the Vulkan physical device
//
void VulkanContext::createPhysicalDevice(VulkanContextCreateInfo info, VkSurfaceKHR surface) {
	std::vector<VkPhysicalDevice> compatiblePhysicalDevices = getCompatiblePhysicalDevices(info, surface);
	if (compatiblePhysicalDevices.size() > 0) m_physicalDevice = compatiblePhysicalDevices[0];
	else throw std::runtime_error("Cannot find any compatible physical device");
}

//--------------------------------------------------------------------------------------------------
// Create the Vulkan logical device with given device extensions and physical device features enabled, and create the queues
//
void VulkanContext::createDevice(VulkanContextCreateInfo info) {
	//Initialize queue create infos
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies;
	if (m_queueFamilyIndices.isComplete_GTCP()) {
		uniqueQueueFamilies = {
			m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.computeFamily.value(),
			m_queueFamilyIndices.transferFamily.value(), m_queueFamilyIndices.presentFamily.value()
		};
	}
	else
	{
		uniqueQueueFamilies = {
			m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.computeFamily.value(),
			m_queueFamilyIndices.transferFamily.value()
		};
	}
	float queuePriority = 1.0f; // TODO compute queue priority
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//Initialize Vulkan logical device create info
	VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data(); // Add queue create infos
	createInfo.pEnabledFeatures = nullptr;
	createInfo.pNext = &m_physicalFeaturesStructChain; // When version >= vulkan1.1 we use pNext to add physical device features
	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = m_deviceExtensions.data(); // Add device extensions
	if (m_enableValidationLayers) {
		//Add instance layers
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_instanceLayers.size());
		createInfo.ppEnabledLayerNames = m_instanceLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Create logical device
	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	//Create queues
	VulkanDebugUtil debugUtil{ m_instance, m_device};
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
	debugUtil.setObjectName(m_graphicsQueue, "GTC Queue");
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.computeFamily.value(), 0, &m_computeQueue);
	debugUtil.setObjectName(m_computeQueue, "Compute Queue");
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.transferFamily.value(), 0, &m_transferQueue);
	debugUtil.setObjectName(m_transferQueue, "Transfer Queue");
	if (m_queueFamilyIndices.isComplete_GTCP()) {
		vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue);
		debugUtil.setObjectName(m_presentQueue, "Present Queue");
	}
	
}

//--------------------------------------------------------------------------------------------------
// Select the physical device that meets the requirements
//
std::vector<VkPhysicalDevice> VulkanContext::getCompatiblePhysicalDevices(VulkanContextCreateInfo info, VkSurfaceKHR surface) {
	
	//Check if there are any physical device available
	uint32_t availableDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &availableDeviceCount, nullptr);
	if (availableDeviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> compatibleDevices{};
	std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
	vkEnumeratePhysicalDevices(m_instance, &availableDeviceCount, availableDevices.data());

	//Find all suitable devices	
	constructStructChains(info);//Construct struct chains
	for (const VkPhysicalDevice device : availableDevices) {
		if (isDeviceSuitable(device, info, surface)) {
			compatibleDevices.push_back(device);
			break;
		}
	}

	if (compatibleDevices.size() == 0) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return compatibleDevices;
}

//--------------------------------------------------------------------------------------------------
// Check whether the current physical device is suitable for SceneEditor, check presentation support if a surface is assigned
//
bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device, VulkanContextCreateInfo info, VkSurfaceKHR surface) {
	m_queueFamilyIndices = findQueueFamilies(device, surface); //Initialize queue family indices and check queue family support
	bool extensionsSupported = checkDeviceExtensionSupport(device, info.deviceExtensions); //Check device extension support

	//Check presentation support if surface is assigned
	bool presentSupported = true;
	if (surface) {
		uint32_t formatCount; 
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		presentSupported = formatCount > 0 && presentModeCount > 0;
	}

	//Check physical device features support
	vkGetPhysicalDeviceFeatures2(device, &m_physicalFeaturesStructChain);
	bool requiredFeaturesSupported = checkDeviceFeaturesSupport(device, info.physicalDeviceFeatureRequirements);

	bool deviceSuitable = false;
	//Check presentation support if surface is provided
	if (surface) {
		deviceSuitable = extensionsSupported && requiredFeaturesSupported && m_queueFamilyIndices.isComplete_GTCP() && presentSupported;
	}
	else
	{
		deviceSuitable = extensionsSupported && requiredFeaturesSupported && m_queueFamilyIndices.isComplete_GTC();
	}
	return deviceSuitable;
}

//--------------------------------------------------------------------------------------------------
// Check whether the API support the required Vulkan validation layers
//
bool VulkanContext::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_instanceLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------------------------
// Return the queue families supported by given physical device
//
QueueFamilyIndices  VulkanContext::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			m_queueFamilyIndices.graphicsFamily = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			m_queueFamilyIndices.computeFamily = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			m_queueFamilyIndices.transferFamily = i;
		}

		//If a surface is assigned, check present support
		if (surface) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				m_queueFamilyIndices.presentFamily = i;
			}

			if (m_queueFamilyIndices.isComplete_GTCP()) break;
		}
		else if (m_queueFamilyIndices.isComplete_GTC()) break;
		i++;
	}

	return m_queueFamilyIndices;
}

//--------------------------------------------------------------------------------------------------
// Check whether the required extensions are supported by the physical device
//
bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> deviceExtensions) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

//--------------------------------------------------------------------------------------------------
// Construct the struct chains for physical device features
//
void VulkanContext::constructStructChains(VulkanContextCreateInfo info) {
	m_physicalFeaturesStructChain.features = m_features10;
	// Only support Vulkan 1.2 for now
	if (info.apiMajor == 1 && info.apiMinor >= 2)
	{
		m_physicalFeaturesStructChain.pNext = &m_features11;
		m_features11.pNext = &m_features12;
		m_features12.pNext = nullptr;
	}
	// use the physicalFeaturesStructChain to append extensions
	if (!m_EXTPhysicalDeviceFeatureStructs.empty())
	{
		// build up chain of all used extension features
		for (size_t i = 0; i < m_EXTPhysicalDeviceFeatureStructs.size(); i++)
		{
			auto* header = reinterpret_cast<ExtensionHeader*>(m_EXTPhysicalDeviceFeatureStructs[i]);
			header->pNext = i < m_EXTPhysicalDeviceFeatureStructs.size() - 1 ? m_EXTPhysicalDeviceFeatureStructs[i + 1] : nullptr;
		}

		// append to the end of current feature2 struct
		ExtensionHeader* lastCoreFeature = (ExtensionHeader*)&m_physicalFeaturesStructChain;
		while (lastCoreFeature->pNext != nullptr)
		{
			lastCoreFeature = (ExtensionHeader*)lastCoreFeature->pNext;
		}
		lastCoreFeature->pNext = m_EXTPhysicalDeviceFeatureStructs[0];
	}
}

//--------------------------------------------------------------------------------------------------
// Check whether the required features are supported by the physical device
//
bool VulkanContext::checkDeviceFeaturesSupport(VkPhysicalDevice device, std::map<VkStructureType, std::vector<const char*>> physicalDeviceFeatureRequirements) {
	bool res = true;
	ExtensionHeader* pStructChainIterator = reinterpret_cast<ExtensionHeader*>(&m_physicalFeaturesStructChain);
	while (pStructChainIterator != nullptr) {
		VkStructureType sType = pStructChainIterator->sType;
		if (physicalDeviceFeatureRequirements.find(sType) != physicalDeviceFeatureRequirements.end()) {
			res = res && checkDeviceFeaturesSupport(device, pStructChainIterator, physicalDeviceFeatureRequirements[sType]);
		}
		pStructChainIterator = (ExtensionHeader*)pStructChainIterator->pNext;
	}
	return true;
}

//--------------------------------------------------------------------------------------------------
// Check whether the required features of certgain feature structure are supported by the physical device
//
bool VulkanContext::checkDeviceFeaturesSupport(VkPhysicalDevice device, void* pFeatures, std::vector<const char*> requiredFeatures) {
	std::string structName = _utils.getVkBool32StructName(pFeatures);
	bool res = true;
	for (const char* featureName : requiredFeatures) {
		res = res && _utils.getVkBool32StructValue(pFeatures, featureName);
	}
	return res;
}
}