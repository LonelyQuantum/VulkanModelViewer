#include "vulkan_app_base.h"

//Inital glfw window size
const uint32_t VulkanAppBase::defaultWidth = 1080;
const uint32_t VulkanAppBase::defaultHeight = 720;

/**
* Test app base
*/

//--------------------------------------------
// Test the base class of Vulkan window applications
//
void VulkanAppBase::appBaseTestRun() {
	createVulkanContext();
	createSwapchain();
}

/**
* Basic Vulkan setup
*/

//--------------------------------------------
// Create the Vulkan context, including vulkan instance, physical device, logical device,
// queue family indices, queues and some helpers
//
void VulkanAppBase::createVulkanContext() {
	//Create window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vulkan Model Viewer", nullptr, nullptr);

	//Setup vulkan context
	vkimpl::VulkanContextCreateInfo contextCreateInfo;
	contextCreateInfo.setApiVersion(1, 2);//Set Vulkan API Version
	contextCreateInfo.addInstanceLayer("VK_LAYER_KHRONOS_validation");//Add validation layer
	//Add instance extenesions
	uint32_t count{ 0 };
	const char** reqExtensions = glfwGetRequiredInstanceExtensions(&count);
	for (uint32_t ext_id = 0; ext_id < count; ext_id++)  // Add glfw instance extensions
		contextCreateInfo.addInstanceExtension(reqExtensions[ext_id]);
	contextCreateInfo.addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);// Add debug utils instance extensions
	//Add device extensions and corresponding physical device feature structs, enabling raytracing
	contextCreateInfo.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	contextCreateInfo.addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelFeature);
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
	contextCreateInfo.addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rtPipelineFeature);  // To use vkCmdTraceRaysKHR
	contextCreateInfo.addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);  // Required by ray tracing pipeline
	//Add feature requirements
	contextCreateInfo.addPhysicalDeviceFeatureRequirement(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, "samplerAnisotropy");
	contextCreateInfo.addPhysicalDeviceFeatureRequirement(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, "multiviewGeometryShader");
	contextCreateInfo.addPhysicalDeviceFeatureRequirement(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, "imagelessFramebuffer");

	// Init Vulkan instance
	vkimpl::VulkanContext context;
	context.createInstance(contextCreateInfo);
	glfwCreateWindowSurface(context.m_instance, m_window, nullptr, &m_surface);
	std::vector<VkPhysicalDevice> compatiblePhysicalDevices = context.getCompatiblePhysicalDevices(contextCreateInfo, m_surface);
	context.m_physicalDevice = compatiblePhysicalDevices[0];
	context.createDevice(contextCreateInfo);

	//Vulkan context components
	m_instance = context.m_instance;
	m_physicalDevice = context.m_physicalDevice;
	m_device = context.m_device;
	m_queueFamilyIndices = context.m_queueFamilyIndices;
	m_graphicsQueue = context.m_graphicsQueue;
	m_presentQueue = context.m_presentQueue;
	m_computeQueue = context.m_computeQueue;
	m_transferQueue = context.m_transferQueue;
	m_debugMessenger = context.m_debugMessenger;

	//Vulkan helper
	m_debugUtil = vkimpl::VulkanDebugUtil(m_instance, m_device);
	m_commandUtil = vkimpl::VulkanCommands(m_device);
	m_imageUtil = vkimpl::VulkanImages(m_physicalDevice, m_device);
	m_bufferUtil = vkimpl::VulkanBuffers(m_physicalDevice, m_device);
	m_renderPassUtil = vkimpl::VulkanRenderPass(m_device);
	m_descriptorUtil = vkimpl::VulkanDescriptorSets(m_device);
	m_pipelineUtil = vkimpl::VulkanPipeline(m_device);
}

//--------------------------------------------
// Create the Vulkan swapchain
//
void VulkanAppBase::createSwapchain() {
	//Initialize the Vulkan swapchain
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
	vkimpl::VulkanSwapchain swapchain{ m_physicalDevice, m_surface, m_device, m_queueFamilyIndices };
	swapchain.init(width, height);

	m_swapchain = swapchain.m_swapchain;
	m_swapchainImageNum = swapchain.m_swapchainImages.size();
	m_swapchainImageFormat = swapchain.m_swapchainImageFormat;
	m_swapchainExtent = swapchain.m_swapchainExtent;
	m_swapchainImages = swapchain.m_swapchainImages;
	m_swapchainImageViews = swapchain.m_swapchainImageViews;
	
}


/**
* Basic Vulkan helpers
*/

//--------------------------------------------------------------------------------------------------
// Select a depth format supported by given physical device
//
VkFormat VulkanAppBase::findDepthFormat(VkPhysicalDevice physicalDevice) {
	return findSupportedFormat(
		physicalDevice,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

//--------------------------------------------------------------------------------------------------
// Select a format supported by cuurent physical device with certain flags
//
VkFormat VulkanAppBase::findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}
