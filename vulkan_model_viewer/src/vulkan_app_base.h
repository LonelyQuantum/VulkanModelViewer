#include "vulkan_common.h"
#include "vulkan_debug_util.h"
#include "vulkan_common_utils.h"

#include "vulkan_context.h"
#include "vulkan_swapchain.h"

#include "vulkan_commands.h"
#include "vulkan_images.h"
#include "vulkan_buffers.h"
#include "vulkan_renderpass.h"
#include "vulkan_descriptorsets.h"
#include "vulkan_pipelines.h"

#include "GLFW/glfw3.h"

//--------------------------------------------------------------------------------------------------
// Window application base class with vulkan backend and glfw surface
//
class VulkanAppBase {
public:
	VulkanAppBase(uint32_t width = defaultWidth, uint32_t height = defaultHeight)
		:m_windowWidth(width), m_windowHeight(height) {};
	void appBaseTestRun();

	//window and surface
	GLFWwindow* m_window;
	VkSurfaceKHR m_surface;
	uint32_t m_windowWidth;
	uint32_t m_windowHeight;

	//Vulkan instance, device and physical device
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	//Vulkan queues and queue family index
	vkimpl::QueueFamilyIndices m_queueFamilyIndices;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkQueue m_computeQueue;
	VkQueue m_transferQueue;

	//Vulkan swapchain
	uint32_t m_swapchainImageNum;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;
	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;

	//Helpers
	vkimpl::VulkanDebugUtil m_debugUtil;
	vkimpl::VulkanCommands m_commandUtil;
	vkimpl::VulkanImages m_imageUtil;
	vkimpl::VulkanBuffers m_bufferUtil;
	vkimpl::VulkanDescriptorSets m_descriptorUtil;
	vkimpl::VulkanPipeline m_pipelineUtil;
	vkimpl::VulkanRenderPass m_renderPassUtil;

protected:
	void createVulkanContext();
	void createSwapchain();

	//Basic Vulkan backend helpers
	VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);
	VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	static const uint32_t defaultWidth;
	static const uint32_t defaultHeight;
};