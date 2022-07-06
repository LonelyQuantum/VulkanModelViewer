#include "vulkan_model_viewer.h"

//Constanats
const float PI = 3.141159f;

//Default settings
#ifdef SOURCE_DIR
const std::string SOURCE_PATH = std::string(SOURCE_DIR);
#endif // SOURCE_DIR

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::string SCENE_VERT_SHADER_PATH = SOURCE_PATH + "shaders/scene.vert.glsl.spv";
const std::string SCENE_FRAG_SHADER_PATH = SOURCE_PATH + "shaders/scene.frag.glsl.spv";

const std::string SCENE_NO_LIHGTING_VERT_SHADER_PATH = SOURCE_PATH + "shaders/scene_no_lighting.vert.glsl.spv";
const std::string SCENE_NO_LIHGTING_FRAG_SHADER_PATH = SOURCE_PATH + "shaders/scene_no_lighting.frag.glsl.spv";

const std::string SCENE_WIREFRAME_VERT_SHADER_PATH = SOURCE_PATH + "shaders/wireframe.vert.glsl.spv";
const std::string SCENE_WIREFRAME_FRAG_SHADER_PATH = SOURCE_PATH + "shaders/wireframe.frag.glsl.spv";

const std::string SHADOW_MAPPING_VERT_SHADER_PATH = SOURCE_PATH + "shaders/shadow_mapping.vert.glsl.spv";
const std::string SHADOW_MAPPING_FRAG_SHADER_PATH = SOURCE_PATH + "shaders/shadow_mapping.frag.glsl.spv";

/**
* run
*/

//--------------------------------------------
// Run the Vulkan test app
//
void VulkanModelViewer::run() {
	init();
	mainLoop();
	cleanup();
}

//--------------------------------------------
// Initialize application
//
void VulkanModelViewer::init() {
	initAppSettings();
	initVulkan();
	initAppResources();
}

//--------------------------------------------
// Main loop of the app
//
void VulkanModelViewer::mainLoop() {
	while (!glfwWindowShouldClose(m_window)) {
		glfwPollEvents();
		handleInput();
		setGuiComponents();
		update();
		drawFrame();
	}

	vkDeviceWaitIdle(m_device);
}

//--------------------------------------------
// Clean up all application resources
//
void VulkanModelViewer::cleanup() {
	cleanupVulkanBackend();
}


/**
* Application layer
*/

//--------------------------------------------
// Initialize frontend applicaiton settings
//
void VulkanModelViewer::initAppSettings() {
	m_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	m_mipLevel = 6;
	m_shadowMapExtent = { 4096, 4096 };
	m_sceneClearColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	_camera.pos = { 0.0f, 0.0f, 1.0f };
	_camera.lookDir = { 0.0f, 0.0f, -1.0f };
	_camera.upDir = { 0.0f, 0.0f, 1.0f };
	_lightSource.pos = { 10.0f, 0.0f, 0.0f };
	_lightSource.color = { 2.0f, 2.0f, 2.0f };
}

//--------------------------------------------
// Initialize frontend applicaiton resources
//
void VulkanModelViewer::initAppResources() {
	initDefaultMaterial();
	glfwSetScrollCallback(m_window, glfwScrollCallback);
}

//--------------------------------------------
// Initialize default material to be white
//
void VulkanModelViewer::initDefaultMaterial() {
	//Create empty material
	Material defaultMat = {};
	defaultMat.diffuse = { 0.5f, 0.5f, 0.5f };
	updateMaterialUbo(defaultMat);

	_materialCache.push_back(defaultMat);
	_uniformBuffers.materialUniformBuffers.push_back(getMaterialUniformBuffer(defaultMat));
	_descriptorSets.materialDescriptorSets.push_back(getMaterialDescriptorSet(defaultMat, _uniformBuffers.materialUniformBuffers.back().buffer));
}





/**
* Vulkan main renderer layer
*/

//--------------------------------------------
// Initialize Vulkan backends for major renderer
//
void VulkanModelViewer::initVulkan() {
	createVulkanContext();
	createSwapchain();
	initCommandPools();
	initRenderSettings();
	initRenderPasses();
	initImageResources();
	initFramebuffers();
	initUniformBuffers();
	initDescriptorSetLayouts();
	initDescriptorPools();
	createSamplers();
	initPipelines();
	initDescriptorSets();
	initCommandBuffers();
	initSyncObjects();
	beginRenderPasses();
	initGuiBackend();
}

//--------------------------------------------
// Initialize the format and other settings for rendering
//
void VulkanModelViewer::initRenderSettings() {
	_defaultDepthFormat = findDepthFormat(m_physicalDevice);
}




//--------------------------------------------
// Create renderpasses used for this application
//
void VulkanModelViewer::initRenderPasses() {
	createPresentRenderPasses();
	createShadowRenderPass();
}

//--------------------------------------------
// Create renderpasses for present
//
void VulkanModelViewer::createPresentRenderPasses() {
	createSceneRenderPass();
	createWireframeRenderPass();
	createGuiRenderPass();
}

//--------------------------------------------
// Create renderpasses for the 3D scene
//
void VulkanModelViewer::createSceneRenderPass() {
	vkimpl::VulkanRenderPassCreateInfo renderPassCreateInfo{ true, true, true };
	renderPassCreateInfo.colorAttachment.format = m_swapchainImageFormat;
	renderPassCreateInfo.colorAttachment.samples = m_msaaSamples;
	renderPassCreateInfo.depthAttachment.format = _defaultDepthFormat;
	renderPassCreateInfo.depthAttachment.samples = m_msaaSamples;
	renderPassCreateInfo.colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	_renderPasses.sceneRenderPass = m_renderPassUtil.createRenderPass(renderPassCreateInfo);
	m_debugUtil.setObjectName(_renderPasses.sceneRenderPass, "SceneRenderPass");
}

//--------------------------------------------
// Create renderpasses for the 3D wireframe
//
void VulkanModelViewer::createWireframeRenderPass() {
	vkimpl::VulkanRenderPassCreateInfo renderPassCreateInfo{ true, true, true };
	renderPassCreateInfo.colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	renderPassCreateInfo.colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	renderPassCreateInfo.colorAttachment.format = m_swapchainImageFormat;
	renderPassCreateInfo.colorAttachment.samples = m_msaaSamples;
	renderPassCreateInfo.depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	renderPassCreateInfo.depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	renderPassCreateInfo.depthAttachment.format = _defaultDepthFormat;
	renderPassCreateInfo.depthAttachment.samples = m_msaaSamples;
	renderPassCreateInfo.colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	_renderPasses.wireframeRenderPass = m_renderPassUtil.createRenderPass(renderPassCreateInfo);
	m_debugUtil.setObjectName(_renderPasses.wireframeRenderPass, "WireframeRenderPass");
}

//--------------------------------------------
// Create renderpasses for the shadow mapping
//
void VulkanModelViewer::createShadowRenderPass() {
	vkimpl::VulkanRenderPassCreateInfo renderPassInfoShadow{ false, true, false };
	renderPassInfoShadow.depthAttachment.format = _defaultDepthFormat;
	renderPassInfoShadow.depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassInfoShadow.depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	renderPassInfoShadow.depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	_renderPasses.shadowRenderPass = m_renderPassUtil.createRenderPass(renderPassInfoShadow);
	m_debugUtil.setObjectName(_renderPasses.shadowRenderPass, "ShadowRenderPass");
}

//--------------------------------------------
// Create renderpasses for GUI
//
void VulkanModelViewer::createGuiRenderPass() {
	vkimpl::VulkanRenderPassCreateInfo renderPassCreateInfo{ true, false, false };
	renderPassCreateInfo.colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	renderPassCreateInfo.colorAttachment.format = m_swapchainImageFormat;
	renderPassCreateInfo.colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	renderPassCreateInfo.colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	renderPassCreateInfo.dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	renderPassCreateInfo.dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	_renderPasses.guiRenderPass = m_renderPassUtil.createRenderPass(renderPassCreateInfo);
	m_debugUtil.setObjectName(_renderPasses.guiRenderPass, "GUIRenderPass");
}




//--------------------------------------------
// Create the color and depth images and image views for framebuffers
//
void VulkanModelViewer::initImageResources() {
	createPresentImageResources();

	//Shadow image
	vkimpl::VulkanImageInfo shadowDepthInfo = getImageInfo(DEPTH_IMAGE);
	shadowDepthInfo.extent.width = m_shadowMapExtent.width;
	shadowDepthInfo.extent.height = m_shadowMapExtent.height;
	shadowDepthInfo.numSamples = VK_SAMPLE_COUNT_1_BIT;
	shadowDepthInfo.usage = shadowDepthInfo.usage | VK_IMAGE_USAGE_SAMPLED_BIT;
	_imageResources.shadowDepth = getImageResource(shadowDepthInfo);
	m_debugUtil.setObjectName(_imageResources.shadowDepth.image, "shadowDepthImage");
	m_debugUtil.setObjectName(_imageResources.shadowDepth.imageMemory, "shadowDepthImageMemory");
	m_debugUtil.setObjectName(_imageResources.shadowDepth.imageView, "shadowDepthImageView");

	//Default shadow image
	vkimpl::VulkanImageInfo defaultShadowDepthInfo = getImageInfo(DEPTH_IMAGE);
	defaultShadowDepthInfo.extent.width = m_shadowMapExtent.width;
	defaultShadowDepthInfo.extent.height = m_shadowMapExtent.height;
	defaultShadowDepthInfo.numSamples = VK_SAMPLE_COUNT_1_BIT;
	defaultShadowDepthInfo.usage = defaultShadowDepthInfo.usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	std::vector<float> pixels(m_shadowMapExtent.width * m_shadowMapExtent.height * 4, 1.0f);
	VkDeviceSize imageSize = pixels.size();
	m_imageUtil.setOperationInfo(_commandPool, m_graphicsQueue, defaultShadowDepthInfo);
	m_imageUtil.createImage(_imageResources.defaultShadowDepth.image, _imageResources.defaultShadowDepth.imageMemory);
	m_imageUtil.fillImagePixels(_imageResources.defaultShadowDepth.image, pixels.data(), imageSize, VK_IMAGE_LAYOUT_UNDEFINED, defaultShadowDepthInfo.aspectFlags);
	m_imageUtil.transitionImageLayout(_imageResources.defaultShadowDepth.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	_imageResources.defaultShadowDepth.imageView = m_imageUtil.createImageView(_imageResources.defaultShadowDepth.image);//Create the texture image view

	m_debugUtil.setObjectName(_imageResources.defaultShadowDepth.image, "shadowDepthImage");
	m_debugUtil.setObjectName(_imageResources.defaultShadowDepth.imageMemory, "shadowDepthImageMemory");
	m_debugUtil.setObjectName(_imageResources.defaultShadowDepth.imageView, "shadowDepthImageView");

	//Create the empty texture and change its layout to shader optimal
	vkimpl::VulkanImageInfo emptyTextureInfo = getImageInfo(TEXTURE_IMAGE);
	emptyTextureInfo.extent.width = 1;
	emptyTextureInfo.extent.height = 1;
	emptyTextureInfo.numSamples = VK_SAMPLE_COUNT_1_BIT;
	emptyTextureInfo.usage = emptyTextureInfo.usage | VK_IMAGE_USAGE_SAMPLED_BIT;
	ImageResource emptyTexture{};
	m_imageUtil.setOperationInfo(_commandPool, m_graphicsQueue, emptyTextureInfo);
	m_imageUtil.createImage(emptyTexture.image, emptyTexture.imageMemory);
	m_imageUtil.transitionImageLayout(emptyTexture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	emptyTexture.imageView = m_imageUtil.createImageView(emptyTexture.image);
	_textureResources.push_back(emptyTexture);
	_texturePaths.push_back("empty_texture");
	m_debugUtil.setObjectName(_textureResources[0].image, "emptyTextureImage");
	m_debugUtil.setObjectName(_textureResources[0].imageMemory, "emptyTextureImageMemory");
	m_debugUtil.setObjectName(_textureResources[0].imageView, "emptyTextureImageView");
}

//--------------------------------------------
// Create the color and depth images and image views for present
//
void VulkanModelViewer::createPresentImageResources() {
	vkimpl::VulkanImageInfo sceneColorInfo = getImageInfo(COLOR_IMAGE);
	_imageResources.sceneColor = getImageResource(sceneColorInfo);
	m_debugUtil.setObjectName(_imageResources.sceneColor.image, "sceneColorImage");
	m_debugUtil.setObjectName(_imageResources.sceneColor.imageMemory, "sceneColorImageMemory");
	m_debugUtil.setObjectName(_imageResources.sceneColor.imageView, "sceneColorImageView");

	vkimpl::VulkanImageInfo sceneDepthInfo = getImageInfo(DEPTH_IMAGE);
	_imageResources.sceneDepth = getImageResource(sceneDepthInfo);
	m_debugUtil.setObjectName(_imageResources.sceneDepth.image, "sceneDepthImage");
	m_debugUtil.setObjectName(_imageResources.sceneDepth.imageMemory, "sceneDepthImageMemory");
	m_debugUtil.setObjectName(_imageResources.sceneDepth.imageView, "sceneDepthImageView");
}

//--------------------------------------------
// Create texture images
//
VulkanModelViewer::ImageResource VulkanModelViewer::createTextureImageResource(std::string texPath) {
	//Load texture image data
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texPath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;

	//Create the texture image and fill its data
	vkimpl::VulkanImageInfo textureImageInfo = getImageInfo(TEXTURE_IMAGE);
	textureImageInfo.extent.width = texWidth;
	textureImageInfo.extent.height = texHeight;
	textureImageInfo.mipLevels = mipLevels;
	m_imageUtil.setOperationInfo(_commandPool, m_graphicsQueue, textureImageInfo);
	m_imageUtil.createImage(textureImage, textureImageMemory);
	m_imageUtil.fillImagePixels(textureImage, pixels, imageSize, VK_IMAGE_LAYOUT_UNDEFINED, textureImageInfo.aspectFlags);
	stbi_image_free(pixels); // free image data after the filling the image
	m_imageUtil.generateMipmaps(textureImage);

	textureImageView = m_imageUtil.createImageView(textureImage);//Create the texture image view

	return ImageResource{ textureImage, textureImageMemory, textureImageView };
}




//--------------------------------------------
// Initialize the scene buffers
//
void VulkanModelViewer::initSceneResources() {
	_indices = {};
	_vertices = {};
	createModelBuffer();
}

//--------------------------------------------
// Create the vertex and index buffer from model vertices and indices
//
void VulkanModelViewer::createModelBuffer() {
	m_bufferUtil.m_commandPool = _commandPool;
	m_bufferUtil.m_queue = m_graphicsQueue;
	//Create the vertex buffer
	VkDeviceSize vertexBufferSize = sizeof(_vertices[0]) * _vertices.size();
	m_bufferUtil.createBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);
	m_bufferUtil.fillBufferData(_vertexBuffer, _vertices.data(), vertexBufferSize);
	//Create the index buffer
	VkDeviceSize indexBufferSize = sizeof(_indices[0]) * _indices.size();
	m_bufferUtil.createBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indexBuffer, _indexBufferMemory);
	m_bufferUtil.fillBufferData(_indexBuffer, _indices.data(), indexBufferSize);
}





//--------------------------------------------
// Create frame buffers
//
void VulkanModelViewer::initFramebuffers() {
	createPresentFramebuffers();
	createShadowFramebuffers();
	
}

//--------------------------------------------
// Create frame buffers for present
//
void VulkanModelViewer::createPresentFramebuffers() {
	createSceneFramebuffers();
	createGuiFramebuffers();
}

//--------------------------------------------
// Create frame buffers for scene rendering
//
void VulkanModelViewer::createSceneFramebuffers() {
	std::vector<std::string> framebufferNames(m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		framebufferNames[i] = "SceneFrameBuffer[" + std::to_string(i) + "]";
	}
	createFramebuffers(_renderPasses.sceneRenderPass, m_swapchainExtent, { _imageResources.sceneColor.imageView, _imageResources.sceneDepth.imageView }, { m_swapchainImageViews }, framebufferNames, _sceneFramebuffers);
}

//--------------------------------------------
// Create frame buffers for gui rendering
//
void VulkanModelViewer::createGuiFramebuffers() {
	std::vector<std::string> framebufferNames(m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		framebufferNames[i] = "GuiFrameBuffer[" + std::to_string(i) + "]";
	}
	createFramebuffers(_renderPasses.guiRenderPass, m_swapchainExtent, {}, { m_swapchainImageViews }, framebufferNames, _guiFramebuffers);
}

//--------------------------------------------
// Create frame buffers for gui rendering
//
void VulkanModelViewer::createShadowFramebuffers() {
	std::vector<std::string> framebufferNames(m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		framebufferNames[i] = "ShadowFrameBuffer[" + std::to_string(i) + "]";
	}
	createFramebuffers(_renderPasses.shadowRenderPass, m_shadowMapExtent, { _imageResources.shadowDepth.imageView }, {}, framebufferNames, _shadowFramebuffers);
}

//--------------------------------------------
// Create frame buffers given render pass and attachments
//
void VulkanModelViewer::createFramebuffers(VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews, std::vector<std::vector<VkImageView>> imageViewArrays, std::vector<std::string> framebufferNames, std::vector<VkFramebuffer>& framebuffers) {
	framebuffers.resize(m_swapchainImageNum);
	int attachmentCount = imageViews.size() + imageViewArrays.size();

	std::vector<VkImageView> attachments(attachmentCount);
	int reusedAttachmentCount = imageViews.size();
	for (int i = 0; i < reusedAttachmentCount; i++)
		attachments[i] = imageViews[i];
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentCount);

	for (size_t i = 0; i < m_swapchainImageNum; i++) {
		for (int j = reusedAttachmentCount; j < attachmentCount; j++)
			attachments[j] = imageViewArrays[j - reusedAttachmentCount][i];
		framebufferInfo.pAttachments = attachments.data();
		if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
		m_debugUtil.setObjectName(framebuffers[i], framebufferNames[i]);
	}
}




//--------------------------------------------------------------------------------------------------
// Initialize the uniform buffers
//
void VulkanModelViewer::initUniformBuffers() {
	createPresentUniformBuffers();
}

//--------------------------------------------------------------------------------------------------
// Initialize the uniform buffers used in presentation
//
void VulkanModelViewer::createPresentUniformBuffers() {
	//Camera info ubo
	createUniformBuffers(sizeof(CameraInfoUBO), _uniformBuffers.cameraUniformBuffers);

	//Light info ubo
	createUniformBuffers(sizeof(LightInfoUBO), _uniformBuffers.lightUniformBuffers);
}

//--------------------------------------------------------------------------------------------------
// Initialize the uniform buffers of given size
//
void VulkanModelViewer::createUniformBuffers(VkDeviceSize bufferSize, std::vector<BufferResource>& uniformBuffers) {
	uniformBuffers.resize(m_swapchainImages.size());
	for (size_t i = 0; i < m_swapchainImages.size(); i++)
		m_bufferUtil.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer, uniformBuffers[i].bufferMemory);
	
}




//--------------------------------------------------------------------------------------------------
// Initialize the descriptor layouts required for rendering
//
void VulkanModelViewer::initDescriptorSetLayouts() {
	createSceneDescriptorSetLayout();
	createCameraDescriptorSetLayout();
	createMaterialDescriptorSetLayout();
	createLightDescriptorSetLayout();
}

//--------------------------------------------------------------------------------------------------
// create the descriptor set layouts used for rendering the scene
//
void VulkanModelViewer::createSceneDescriptorSetLayout() {
	//Binding infos
	std::vector<vkimpl::DescriptorSetLayoutBindingInfo> descriptorBindingInfos;
	//Camera information uniform buffer binding
	vkimpl::DescriptorSetLayoutBindingInfo cameraUboDescriptorInfo{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(cameraUboDescriptorInfo);
	//Light information uniform buffer binding
	vkimpl::DescriptorSetLayoutBindingInfo lightUboDescriptorInfo{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(lightUboDescriptorInfo);
	//Shadow texture
	vkimpl::DescriptorSetLayoutBindingInfo shadowTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(shadowTextureEntry);
	

	//Create layout
	_descriptorSetInfos.sceneDescriptorInfo.bindingInfos = descriptorBindingInfos;
	m_descriptorUtil.createDescriptorSetLayout(descriptorBindingInfos, _descriptorSetLayouts.sceneDescriptorSetLayout);
}

//--------------------------------------------------------------------------------------------------
// create the descriptor set layouts used for rendering the scene
//
void VulkanModelViewer::createCameraDescriptorSetLayout() {
	//Binding infos
	std::vector<vkimpl::DescriptorSetLayoutBindingInfo> descriptorBindingInfos;
	//Camera information uniform buffer binding
	vkimpl::DescriptorSetLayoutBindingInfo cameraUboDescriptorInfo{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(cameraUboDescriptorInfo);

	//Create layout
	_descriptorSetInfos.cameraDescriptorInfo.bindingInfos = descriptorBindingInfos;
	m_descriptorUtil.createDescriptorSetLayout(descriptorBindingInfos, _descriptorSetLayouts.cameraDescriptorSetLayout);
}

//--------------------------------------------------------------------------------------------------
// create the descriptor set layouts used for rendering the scene
//
void VulkanModelViewer::createLightDescriptorSetLayout() {
	//Binding infos
	std::vector<vkimpl::DescriptorSetLayoutBindingInfo> descriptorBindingInfos;
	vkimpl::DescriptorSetLayoutBindingInfo lightUboDescriptorInfo{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(lightUboDescriptorInfo);


	//Create layout
	_descriptorSetInfos.lightDescriptorInfo.bindingInfos = descriptorBindingInfos;
	m_descriptorUtil.createDescriptorSetLayout(descriptorBindingInfos, _descriptorSetLayouts.lightDescriptorSetLayout);
}

//--------------------------------------------------------------------------------------------------
// create the descriptor set layouts used for material and textures it has
//
void VulkanModelViewer::createMaterialDescriptorSetLayout() {
	//Binding infos
	std::vector<vkimpl::DescriptorSetLayoutBindingInfo> descriptorBindingInfos{};
	//Material uniform buffer binding
	vkimpl::DescriptorSetLayoutBindingInfo materialUboEntry{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(materialUboEntry);
	vkimpl::DescriptorSetLayoutBindingInfo ambientTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(ambientTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo diffuseTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(diffuseTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo specularTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(specularTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo specularHighlightTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(specularHighlightTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo bumpTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(bumpTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo displacementTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(displacementTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo alphaTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(alphaTextureEntry);
	vkimpl::DescriptorSetLayoutBindingInfo reflectionTextureEntry{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT };
	descriptorBindingInfos.push_back(reflectionTextureEntry);

	//Create layout
	_descriptorSetInfos.materialDescriptorInfo.bindingInfos = descriptorBindingInfos;
	m_descriptorUtil.createDescriptorSetLayout(descriptorBindingInfos, _descriptorSetLayouts.materialDescriptorSetLayout);
}





//--------------------------------------------------------------------------------------------------
// Initialize the descriptor pools for descriptor sets allocation
//
void VulkanModelViewer::initDescriptorPools() {
	createPresentDescriptorPools();
	createGuiDescriptorPool();
	createMaterialDescriptorPool();
}

//--------------------------------------------------------------------------------------------------
// Create descriptor pools for present
//
void VulkanModelViewer::createPresentDescriptorPools() {
	createSceneDescriptorPool();
	createCameraDescriptorPool();
	createLightDescriptorPool();
}

//--------------------------------------------------------------------------------------------------
// Create the scene rendering descriptor pool
//
void VulkanModelViewer::createSceneDescriptorPool() {
	int maxPipelineNums = 3;
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPipelineNums * m_swapchainImageNum},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxPipelineNums * m_swapchainImageNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxPipelineNums * m_swapchainImageNum}
	};
	m_descriptorUtil.createDescriptorPool(maxPipelineNums * m_swapchainImageNum, poolSizes, _descriptorPools.sceneDescriptorPool);
}

//--------------------------------------------------------------------------------------------------
// Create the scene rendering descriptor pool
//
void VulkanModelViewer::createCameraDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_swapchainImageNum},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_swapchainImageNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_swapchainImageNum}
	};
	m_descriptorUtil.createDescriptorPool(m_swapchainImageNum, poolSizes, _descriptorPools.cameraDescriptorPool);
}

//--------------------------------------------------------------------------------------------------
// Create the scene rendering descriptor pool
//
void VulkanModelViewer::createLightDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_swapchainImageNum}
	};
	m_descriptorUtil.createDescriptorPool(m_swapchainImageNum, poolSizes, _descriptorPools.lightDescriptorPool);
}

//--------------------------------------------------------------------------------------------------
// Create the material system descriptor pool
//
void VulkanModelViewer::createMaterialDescriptorPool() {
	uint32_t maxMaterialNum = 1000;
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxMaterialNum},
	};
	m_descriptorUtil.createDescriptorPool(maxMaterialNum, poolSizes, _descriptorPools.materialDescriptorPool);
}

//--------------------------------------------------------------------------------------------------
// Create the gui descriptor pool
//
void VulkanModelViewer::createGuiDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	m_descriptorUtil.createDescriptorPool(1000 * poolSizes.size(), poolSizes, _descriptorPools.guiDescriptorPool);
}




//--------------------------------------------------------------------------------------------------
// Create the samplers for images
//
void VulkanModelViewer::createSamplers() {
	createTextureSampler();
	createShadowSampler();
}

//--------------------------------------------------------------------------------------------------
// Create the sampler of the texture image
//
void VulkanModelViewer::createTextureSampler() {
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f; // Optional
	samplerInfo.maxLod = static_cast<float>(m_mipLevel);
	samplerInfo.mipLodBias = 0.0f; // Optional  

	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &_samplers.textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

//--------------------------------------------------------------------------------------------------
// Create the sampler of the texture image
//
void VulkanModelViewer::createShadowSampler() {
	//Create shadow sampler
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f; // Optional
	samplerInfo.maxLod = 1.0f;
	samplerInfo.mipLodBias = 0.0f; // Optional  

	if (vkCreateSampler(m_device, &samplerInfo, nullptr, &_samplers.shadowSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}





//--------------------------------------------------------------------------------------------------
// Create the command pools for all kinds of command buffers
//
void VulkanModelViewer::initCommandPools() {
	_commandPool = m_commandUtil.createCommandPool(m_queueFamilyIndices.graphicsFamily.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}


//--------------------------------------------------------------------------------------------------
// Create the pipelines
//
void VulkanModelViewer::initPipelines() {
	createPresentPipelines();
	createShadowPipeline();
}

//--------------------------------------------------------------------------------------------------
// Create the pipelines for present
//
void VulkanModelViewer::createPresentPipelines() {
	createScenePipeline();
	createSceneNoLightingPipeline();
	createWireframePipeline();
}

//--------------------------------------------------------------------------------------------------
// Create the pipelines for scene
//
void VulkanModelViewer::createScenePipeline() {
	std::string vertexShaderPath = SCENE_VERT_SHADER_PATH;
	std::string fragmentShaderPath = SCENE_FRAG_SHADER_PATH;

	auto vertShaderCode = readFile(vertexShaderPath);
	auto fragShaderCode = readFile(fragmentShaderPath);

	vkimpl::VulkanPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.vertShaderCode = vertShaderCode;
	graphicsPipelineCreateInfo.fragShaderCode = fragShaderCode;
	graphicsPipelineCreateInfo.bindingDescription = Vertex::getBindingDescription();
	graphicsPipelineCreateInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
	graphicsPipelineCreateInfo.extent = m_swapchainExtent;
	graphicsPipelineCreateInfo.msaaSamples = m_msaaSamples;
	graphicsPipelineCreateInfo.descriptorSetLayouts = { _descriptorSetLayouts.sceneDescriptorSetLayout, _descriptorSetLayouts.materialDescriptorSetLayout };
	graphicsPipelineCreateInfo.renderPass = _renderPasses.sceneRenderPass;

	m_pipelineUtil.initGraphicsPipelineCreateInfo(graphicsPipelineCreateInfo);
	m_pipelineUtil.createGraphicsPipelineLayout(_pipelineLayouts.scenePipelineLayout);
	m_pipelineUtil.createGraphicsPipeline(_pipelines.scenePipeline);
}

//--------------------------------------------------------------------------------------------------
// Create the pipelines for scene without lighting
//
void VulkanModelViewer::createSceneNoLightingPipeline() {
	std::string vertexShaderPath = SCENE_NO_LIHGTING_VERT_SHADER_PATH;
	std::string fragmentShaderPath = SCENE_NO_LIHGTING_FRAG_SHADER_PATH;

	auto vertShaderCode = readFile(vertexShaderPath);
	auto fragShaderCode = readFile(fragmentShaderPath);

	vkimpl::VulkanPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.vertShaderCode = vertShaderCode;
	graphicsPipelineCreateInfo.fragShaderCode = fragShaderCode;
	graphicsPipelineCreateInfo.bindingDescription = Vertex::getBindingDescription();
	graphicsPipelineCreateInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
	graphicsPipelineCreateInfo.extent = m_swapchainExtent;
	graphicsPipelineCreateInfo.msaaSamples = m_msaaSamples;
	graphicsPipelineCreateInfo.descriptorSetLayouts = { _descriptorSetLayouts.sceneDescriptorSetLayout, _descriptorSetLayouts.materialDescriptorSetLayout };
	graphicsPipelineCreateInfo.renderPass = _renderPasses.sceneRenderPass;

	m_pipelineUtil.initGraphicsPipelineCreateInfo(graphicsPipelineCreateInfo);
	m_pipelineUtil.createGraphicsPipelineLayout(_pipelineLayouts.sceneNoLightingPipelineLayout);
	m_pipelineUtil.createGraphicsPipeline(_pipelines.sceneNoLightingPipeline);
}

//--------------------------------------------------------------------------------------------------
// Create the pipelines for scene
//
void VulkanModelViewer::createWireframePipeline() {
	std::string vertexShaderPath = SCENE_WIREFRAME_VERT_SHADER_PATH;
	std::string fragmentShaderPath = SCENE_WIREFRAME_FRAG_SHADER_PATH;

	auto vertShaderCode = readFile(vertexShaderPath);
	auto fragShaderCode = readFile(fragmentShaderPath);

	vkimpl::VulkanPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.vertShaderCode = vertShaderCode;
	graphicsPipelineCreateInfo.fragShaderCode = fragShaderCode;
	graphicsPipelineCreateInfo.bindingDescription = Vertex::getBindingDescription();
	graphicsPipelineCreateInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
	graphicsPipelineCreateInfo.extent = m_swapchainExtent;
	graphicsPipelineCreateInfo.msaaSamples = m_msaaSamples;
	graphicsPipelineCreateInfo.descriptorSetLayouts = { _descriptorSetLayouts.cameraDescriptorSetLayout };
	graphicsPipelineCreateInfo.renderPass = _renderPasses.sceneRenderPass;

	m_pipelineUtil.initGraphicsPipelineCreateInfo(graphicsPipelineCreateInfo);
	m_pipelineUtil.m_rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	m_pipelineUtil.m_rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
	m_pipelineUtil.createGraphicsPipelineLayout(_pipelineLayouts.wireframePipelineLayout);
	m_pipelineUtil.createGraphicsPipeline(_pipelines.wireframePipeline);
}

//--------------------------------------------------------------------------------------------------
// Create the pipelines for shadow mapping
//
void VulkanModelViewer::createShadowPipeline() {
	std::string vertexShaderPath = SHADOW_MAPPING_VERT_SHADER_PATH;
	std::string fragmentShaderPath = SHADOW_MAPPING_FRAG_SHADER_PATH;

	auto vertShaderCode = readFile(vertexShaderPath);
	auto fragShaderCode = readFile(fragmentShaderPath);

	vkimpl::VulkanPipelineCreateInfo graphicsPipelineCreateInfo{};
	graphicsPipelineCreateInfo.vertShaderCode = vertShaderCode;
	graphicsPipelineCreateInfo.fragShaderCode = fragShaderCode;
	graphicsPipelineCreateInfo.bindingDescription = Vertex::getBindingDescription();
	graphicsPipelineCreateInfo.attributeDescriptions = Vertex::getAttributeDescriptions();
	graphicsPipelineCreateInfo.extent = m_shadowMapExtent;
	graphicsPipelineCreateInfo.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	graphicsPipelineCreateInfo.descriptorSetLayouts = { _descriptorSetLayouts.lightDescriptorSetLayout };
	graphicsPipelineCreateInfo.renderPass = _renderPasses.shadowRenderPass;

	m_pipelineUtil.initGraphicsPipelineCreateInfo(graphicsPipelineCreateInfo);
	m_pipelineUtil.createGraphicsPipelineLayout(_pipelineLayouts.shadowPipelineLayout);
	m_pipelineUtil.createGraphicsPipeline(_pipelines.shadowPipeline);
}






//--------------------------------------------------------------------------------------------------
// Initialize the descriptor sets
//
void VulkanModelViewer::initDescriptorSets() {
	createPresentDescriptorSets();
}

//--------------------------------------------------------------------------------------------------
// Initialize the descriptor sets
//
void VulkanModelViewer::createPresentDescriptorSets() {
	createNoShadowSceneDescriptorSets();
	createSceneDescriptorSets();
	createCameraDescriptorSets();
	createLightDescriptorSets();
}

//--------------------------------------------------------------------------------------------------
// Create the descriptor sets for scene information
//
void VulkanModelViewer::createSceneDescriptorSets() {
	_descriptorSetInfos.sceneDescriptorInfo.bufferInfos.clear();
	_descriptorSetInfos.sceneDescriptorInfo.imageInfos.clear();

	std::vector<vkimpl::DescriptorSetInfo> descriptorSetInfos(m_swapchainImageNum, _descriptorSetInfos.sceneDescriptorInfo);
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_swapchainImageNum, _descriptorSetLayouts.sceneDescriptorSetLayout);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		VkDescriptorBufferInfo bufferInfo;
		VkDescriptorImageInfo imageInfo;
		bufferInfo = { _uniformBuffers.cameraUniformBuffers[i].buffer, 0, sizeof(CameraInfoUBO) };
		descriptorSetInfos[i].bufferInfos.push_back(bufferInfo);
		bufferInfo = { _uniformBuffers.lightUniformBuffers[i].buffer, 0, sizeof(LightInfoUBO) };
		descriptorSetInfos[i].bufferInfos.push_back(bufferInfo);
		imageInfo = { _samplers.shadowSampler, _imageResources.shadowDepth.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		descriptorSetInfos[i].imageInfos.push_back(imageInfo);
	}
	m_descriptorUtil.createDescriptorSets(_descriptorPools.sceneDescriptorPool, descriptorSetLayouts, descriptorSetInfos, _descriptorSets.sceneDescriptorSets);
}

//--------------------------------------------------------------------------------------------------
// Create the descriptor sets for scene information with no shadow (a default shadow depth imageview)
//
void VulkanModelViewer::createNoShadowSceneDescriptorSets() {
	_descriptorSetInfos.sceneDescriptorInfo.bufferInfos.clear();
	_descriptorSetInfos.sceneDescriptorInfo.imageInfos.clear();

	std::vector<vkimpl::DescriptorSetInfo> descriptorSetInfos(m_swapchainImageNum, _descriptorSetInfos.sceneDescriptorInfo);
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_swapchainImageNum, _descriptorSetLayouts.sceneDescriptorSetLayout);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		VkDescriptorBufferInfo bufferInfo;
		VkDescriptorImageInfo imageInfo;
		bufferInfo = { _uniformBuffers.cameraUniformBuffers[i].buffer, 0, sizeof(CameraInfoUBO) };
		descriptorSetInfos[i].bufferInfos.push_back(bufferInfo);
		bufferInfo = { _uniformBuffers.lightUniformBuffers[i].buffer, 0, sizeof(LightInfoUBO) };
		descriptorSetInfos[i].bufferInfos.push_back(bufferInfo);
		imageInfo = { _samplers.shadowSampler, _imageResources.defaultShadowDepth.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
		descriptorSetInfos[i].imageInfos.push_back(imageInfo);
	}
	m_descriptorUtil.createDescriptorSets(_descriptorPools.sceneDescriptorPool, descriptorSetLayouts, descriptorSetInfos, _descriptorSets.sceneNoShadowDescriptorSets);
}

//--------------------------------------------------------------------------------------------------
// Create the descriptor sets for wireframe information
//
void VulkanModelViewer::createCameraDescriptorSets() {
	_descriptorSetInfos.cameraDescriptorInfo.bufferInfos.clear();
	_descriptorSetInfos.cameraDescriptorInfo.imageInfos.clear();

	std::vector<vkimpl::DescriptorSetInfo> descriptorSetInfos(m_swapchainImageNum, _descriptorSetInfos.cameraDescriptorInfo);
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_swapchainImageNum, _descriptorSetLayouts.cameraDescriptorSetLayout);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo = { _uniformBuffers.cameraUniformBuffers[i].buffer, 0, sizeof(CameraInfoUBO) };
		descriptorSetInfos[i].bufferInfos.push_back(bufferInfo);
	}
	m_descriptorUtil.createDescriptorSets(_descriptorPools.cameraDescriptorPool, descriptorSetLayouts, descriptorSetInfos, _descriptorSets.cameraDescriptorSets);
}

//--------------------------------------------------------------------------------------------------
// Create the descriptor sets for scene information
//
void VulkanModelViewer::createLightDescriptorSets() {
	_descriptorSetInfos.lightDescriptorInfo.bufferInfos.clear();
	_descriptorSetInfos.lightDescriptorInfo.imageInfos.clear();

	std::vector<vkimpl::DescriptorSetInfo> descriptorSetInfos(m_swapchainImageNum, _descriptorSetInfos.lightDescriptorInfo);
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(m_swapchainImageNum, _descriptorSetLayouts.lightDescriptorSetLayout);
	for (int i = 0; i < m_swapchainImageNum; i++) {
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo = { _uniformBuffers.lightUniformBuffers[i].buffer, 0, sizeof(LightInfoUBO) };
		descriptorSetInfos[i].bufferInfos.push_back(bufferInfo);
	}
	m_descriptorUtil.createDescriptorSets(_descriptorPools.lightDescriptorPool, descriptorSetLayouts, descriptorSetInfos, _descriptorSets.lightDescriptorSets);
}






//--------------------------------------------------------------------------------------------------
// Initialize the command buffers
//
void VulkanModelViewer::initCommandBuffers() {
	createPresentCommandBuffers();
	createShadowCommandBuffers();
}

//--------------------------------------------------------------------------------------------------
// Initialize the command buffers for present
//
void VulkanModelViewer::createPresentCommandBuffers() {
	createDefaultCommandBuffers();
	createSceneCommandBuffers();
	createSceneBlankModelCommandBuffers();
	createNoShadowSceneCommandBuffers();
	createNoShadowSceneBlankModelCommandBuffers();
	createShadowCommandBuffers();
	createWireframeCommandBuffers();
	createGuiCommandBuffers();
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for default scene rendering
//
void VulkanModelViewer::createDefaultCommandBuffers() {
	_commandBuffers.defaultCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.defaultCommandBuffers[i], "DefaultCommandBuffer[" + std::to_string(i) + "]");
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for default scene rendering
//
void VulkanModelViewer::createShadowCommandBuffers() {
	_commandBuffers.shadowCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.shadowCommandBuffers[i], "ShadowCommandBuffer[" + std::to_string(i) + "]");
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for 3D scene rendering
//
void VulkanModelViewer::createSceneCommandBuffers() {
	_commandBuffers.sceneCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.sceneCommandBuffers[i], "SceneCommandBuffer[" + std::to_string(i) + "]");
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for 3D scene rendering without shadow
//
void VulkanModelViewer::createNoShadowSceneCommandBuffers() {
	_commandBuffers.sceneNoShadowCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.sceneNoShadowCommandBuffers[i], "SceneNoShadowCommandBuffer[" + std::to_string(i) + "]");
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for 3D scene rendering with white models
//
void VulkanModelViewer::createSceneBlankModelCommandBuffers() {
	_commandBuffers.sceneBlankModelCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.sceneBlankModelCommandBuffers[i], "SceneBlankModelCommandBuffer[" + std::to_string(i) + "]");
}


//--------------------------------------------------------------------------------------------------
// Create the command buffers for 3D scene rendering with white models and no shadow
//
void VulkanModelViewer::createNoShadowSceneBlankModelCommandBuffers() {
	_commandBuffers.sceneNoShadowBlankModelCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.sceneNoShadowBlankModelCommandBuffers[i], "SceneNoShadowBlankModelCommandBuffer[" + std::to_string(i) + "]");
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for 3D wireframe rendering
//
void VulkanModelViewer::createWireframeCommandBuffers() {
	_commandBuffers.wireframeCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.wireframeCommandBuffers[i], "WireframeCommandBuffer[" + std::to_string(i) + "]");
}

//--------------------------------------------------------------------------------------------------
// Create the command buffers for gui rendering
//
void VulkanModelViewer::createGuiCommandBuffers() {
	_commandBuffers.guiCommandBuffers = m_commandUtil.createCommandBuffers(_commandPool, m_swapchainImageNum);
	for (int i = 0; i < m_swapchainImageNum; i++)
		m_debugUtil.setObjectName(_commandBuffers.guiCommandBuffers[i], "GuiCommandBuffer[" + std::to_string(i) + "]");
}




//--------------------------------------------
// Initialize the semaphores and fences
//
void VulkanModelViewer::initSyncObjects() {
	_maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
	createPresentSyncObjects();
}

//--------------------------------------------
// Create the semaphores and fences for image presentation
//
void VulkanModelViewer::createPresentSyncObjects() {
	_imageAvailableSemaphores.resize(_maxFramesInFlight);
	_renderFinishedSemaphores.resize(_maxFramesInFlight);
	_inFlightFences.resize(_maxFramesInFlight);
	_imagesInFlight.resize(m_swapchainImageNum, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < _maxFramesInFlight; i++) {
		if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}




//--------------------------------------------------------------------------------------------------
// Start the render passes required when starting the application
//
void VulkanModelViewer::beginRenderPasses() {
	beginPresentRenderPasses();
}

//--------------------------------------------------------------------------------------------------
// Start the render passes for present
//
void VulkanModelViewer::beginPresentRenderPasses() {
	beginDefaultRenderPass();
	if (_indices.size() > 0)
		beginObjectRenderPasses();
}

//--------------------------------------------------------------------------------------------------
// Begin the default render pass
//
void VulkanModelViewer::beginDefaultRenderPass() {
	for (size_t i = 0; i < _commandBuffers.defaultCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(_commandBuffers.defaultCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderPasses.sceneRenderPass;
		renderPassInfo.framebuffer = _sceneFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers.defaultCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(_commandBuffers.defaultCommandBuffers[i]);


		if (vkEndCommandBuffer(_commandBuffers.defaultCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Start the render passes for object rendering
//
void VulkanModelViewer::beginObjectRenderPasses() {
	beginNoShadowSceneRenderPass();
	beginSceneRenderPass();
	beginNoShadowSceneBlankModelRenderPass();
	beginSceneBlankModelRenderPass();
	beginShadowRenderPass();
	beginWireframeRenderPass();
}

//--------------------------------------------------------------------------------------------------
// Begin the scene render pass
//
void VulkanModelViewer::beginSceneRenderPass() {
	for (size_t i = 0; i < _commandBuffers.sceneCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(_commandBuffers.sceneCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//Begin the render pass
		VkRenderPassBeginInfo renderPassInfoScene{};
		renderPassInfoScene.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoScene.renderPass = _renderPasses.sceneRenderPass;
		renderPassInfoScene.framebuffer = _sceneFramebuffers[i];
		renderPassInfoScene.renderArea.offset = { 0, 0 };
		renderPassInfoScene.renderArea.extent = m_swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfoScene.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfoScene.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers.sceneCommandBuffers[i], &renderPassInfoScene, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffers.sceneCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines.scenePipeline);
		
		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffers.sceneCommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffers.sceneCommandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		std::vector<VkDescriptorSet> descSets(2);
		descSets[0] = _descriptorSets.sceneDescriptorSets[i];
		for (Shape shape : _shapes) {
			for (MaterialGroup matGroup : shape.materialGroups) {
				int materialId = matGroup.materialId;
				descSets[1] = _descriptorSets.materialDescriptorSets[materialId];
				vkCmdBindDescriptorSets(_commandBuffers.sceneCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayouts.scenePipelineLayout, 0, descSets.size(), descSets.data(), 0, nullptr);
				vkCmdDrawIndexed(_commandBuffers.sceneCommandBuffers[i], matGroup.indexCount, 1, matGroup.indexBase, 0, 0);
			}
		}

		vkCmdEndRenderPass(_commandBuffers.sceneCommandBuffers[i]);


		if (vkEndCommandBuffer(_commandBuffers.sceneCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Begin the scene render pass
//
void VulkanModelViewer::beginNoShadowSceneRenderPass() {
	for (size_t i = 0; i < _commandBuffers.sceneNoShadowCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(_commandBuffers.sceneNoShadowCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//Begin the render pass
		VkRenderPassBeginInfo renderPassInfoScene{};
		renderPassInfoScene.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoScene.renderPass = _renderPasses.sceneRenderPass;
		renderPassInfoScene.framebuffer = _sceneFramebuffers[i];
		renderPassInfoScene.renderArea.offset = { 0, 0 };
		renderPassInfoScene.renderArea.extent = m_swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfoScene.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfoScene.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers.sceneNoShadowCommandBuffers[i], &renderPassInfoScene, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffers.sceneNoShadowCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines.scenePipeline);

		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffers.sceneNoShadowCommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffers.sceneNoShadowCommandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		std::vector<VkDescriptorSet> descSets(2);
		descSets[0] = _descriptorSets.sceneNoShadowDescriptorSets[i];
		for (Shape shape : _shapes) {
			for (MaterialGroup matGroup : shape.materialGroups) {
				int materialId = matGroup.materialId;
				descSets[1] = _descriptorSets.materialDescriptorSets[materialId];
				vkCmdBindDescriptorSets(_commandBuffers.sceneNoShadowCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayouts.scenePipelineLayout, 0, descSets.size(), descSets.data(), 0, nullptr);
				vkCmdDrawIndexed(_commandBuffers.sceneNoShadowCommandBuffers[i], matGroup.indexCount, 1, matGroup.indexBase, 0, 0);
			}
		}

		vkCmdEndRenderPass(_commandBuffers.sceneNoShadowCommandBuffers[i]);


		if (vkEndCommandBuffer(_commandBuffers.sceneNoShadowCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Begin the scene render pass with all models blank
//
void VulkanModelViewer::beginSceneBlankModelRenderPass() {
	for (size_t i = 0; i < _commandBuffers.sceneBlankModelCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(_commandBuffers.sceneBlankModelCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//Begin the render pass
		VkRenderPassBeginInfo renderPassInfoScene{};
		renderPassInfoScene.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoScene.renderPass = _renderPasses.sceneRenderPass;
		renderPassInfoScene.framebuffer = _sceneFramebuffers[i];
		renderPassInfoScene.renderArea.offset = { 0, 0 };
		renderPassInfoScene.renderArea.extent = m_swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfoScene.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfoScene.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers.sceneBlankModelCommandBuffers[i], &renderPassInfoScene, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffers.sceneBlankModelCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines.scenePipeline);

		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffers.sceneBlankModelCommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffers.sceneBlankModelCommandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		std::vector<VkDescriptorSet> descSets(2);
		descSets[0] = _descriptorSets.sceneDescriptorSets[i];
		descSets[1] = _descriptorSets.materialDescriptorSets[0];

		vkCmdBindDescriptorSets(_commandBuffers.sceneBlankModelCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayouts.sceneNoLightingPipelineLayout, 0, descSets.size(), descSets.data(), 0, nullptr);
		vkCmdDrawIndexed(_commandBuffers.sceneBlankModelCommandBuffers[i], _indices.size(), 1, 0, 0, 0);
		vkCmdEndRenderPass(_commandBuffers.sceneBlankModelCommandBuffers[i]);

		if (vkEndCommandBuffer(_commandBuffers.sceneBlankModelCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Begin the scene render pass with all models blank
//
void VulkanModelViewer::beginNoShadowSceneBlankModelRenderPass() {
	for (size_t i = 0; i < _commandBuffers.sceneNoShadowBlankModelCommandBuffers.size(); i++) {
		VkCommandBuffer currentCommandBuffer = _commandBuffers.sceneNoShadowBlankModelCommandBuffers[i];
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(currentCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//Begin the render pass
		VkRenderPassBeginInfo renderPassInfoScene{};
		renderPassInfoScene.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoScene.renderPass = _renderPasses.sceneRenderPass;
		renderPassInfoScene.framebuffer = _sceneFramebuffers[i];
		renderPassInfoScene.renderArea.offset = { 0, 0 };
		renderPassInfoScene.renderArea.extent = m_swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfoScene.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfoScene.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(currentCommandBuffer, &renderPassInfoScene, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines.sceneNoLightingPipeline);

		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(currentCommandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(currentCommandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		std::vector<VkDescriptorSet> descSets(2);
		descSets[0] = _descriptorSets.sceneNoShadowDescriptorSets[i];
		descSets[1] = _descriptorSets.materialDescriptorSets[0];

		vkCmdBindDescriptorSets(currentCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayouts.sceneNoLightingPipelineLayout, 0, descSets.size(), descSets.data(), 0, nullptr);
		vkCmdDrawIndexed(currentCommandBuffer, _indices.size(), 1, 0, 0, 0);
		vkCmdEndRenderPass(currentCommandBuffer);

		if (vkEndCommandBuffer(currentCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Begin the wireframe render pass
//
void VulkanModelViewer::beginWireframeRenderPass() {
	for (size_t i = 0; i < _commandBuffers.wireframeCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(_commandBuffers.wireframeCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//Begin the render pass
		VkRenderPassBeginInfo renderPassInfoScene{};
		renderPassInfoScene.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoScene.renderPass = _renderPasses.wireframeRenderPass;
		renderPassInfoScene.framebuffer = _sceneFramebuffers[i];
		renderPassInfoScene.renderArea.offset = { 0, 0 };
		renderPassInfoScene.renderArea.extent = m_swapchainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {1.0f, 1.0f, 1.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfoScene.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfoScene.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(_commandBuffers.wireframeCommandBuffers[i], &renderPassInfoScene, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffers.wireframeCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines.wireframePipeline);

		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffers.wireframeCommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffers.wireframeCommandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		VkDescriptorSet descSet = _descriptorSets.cameraDescriptorSets[i];
		vkCmdBindDescriptorSets(_commandBuffers.wireframeCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayouts.wireframePipelineLayout, 0, 1, &descSet, 0, nullptr);
		vkCmdDrawIndexed(_commandBuffers.wireframeCommandBuffers[i], _indices.size(), 1, 0, 0, 0);
		vkCmdEndRenderPass(_commandBuffers.wireframeCommandBuffers[i]);

		if (vkEndCommandBuffer(_commandBuffers.wireframeCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Begin the scene render pass
//
void VulkanModelViewer::beginShadowRenderPass() {
	for (size_t i = 0; i < _commandBuffers.shadowCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(_commandBuffers.shadowCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//Begin the render pass
		VkRenderPassBeginInfo renderPassInfoShadow{};
		renderPassInfoShadow.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoShadow.renderPass = _renderPasses.shadowRenderPass;
		renderPassInfoShadow.framebuffer = _shadowFramebuffers[i];
		renderPassInfoShadow.renderArea.offset = { 0, 0 };
		renderPassInfoShadow.renderArea.extent = m_shadowMapExtent;

		VkClearValue clearValue{};
		clearValue.depthStencil = { 1.0f, 0 };

		renderPassInfoShadow.clearValueCount = 1;
		renderPassInfoShadow.pClearValues = &clearValue;

		vkCmdBeginRenderPass(_commandBuffers.shadowCommandBuffers[i], &renderPassInfoShadow, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(_commandBuffers.shadowCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelines.shadowPipeline);

		VkBuffer vertexBuffers[] = { _vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffers.shadowCommandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffers.shadowCommandBuffers[i], _indexBuffer, 0, VK_INDEX_TYPE_UINT32);


		vkCmdBindDescriptorSets(_commandBuffers.shadowCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayouts.shadowPipelineLayout, 0, 1, &_descriptorSets.lightDescriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(_commandBuffers.shadowCommandBuffers[i], _indices.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(_commandBuffers.shadowCommandBuffers[i]);


		if (vkEndCommandBuffer(_commandBuffers.shadowCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

//--------------------------------------------------------------------------------------------------
// Begin the gui render pass
//
void VulkanModelViewer::beginGuiRenderPass(uint32_t imageIndex) {
	// Reset and begin command buffer
	{
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(_commandBuffers.guiCommandBuffers[imageIndex], &info);
	}

	//Begin render pass
	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = _renderPasses.guiRenderPass;
		info.framebuffer = _guiFramebuffers[imageIndex];
		info.renderArea.extent.width = m_swapchainExtent.width;
		info.renderArea.extent.height = m_swapchainExtent.height;
		info.clearValueCount = 1;
		VkClearValue clearValue;
		clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		info.pClearValues = &clearValue;
		vkCmdBeginRenderPass(_commandBuffers.guiCommandBuffers[imageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Record Imgui Draw Data and draw funcs into command buffer
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffers.guiCommandBuffers[imageIndex]);
	// Submit command buffer
	vkCmdEndRenderPass(_commandBuffers.guiCommandBuffers[imageIndex]);
	vkEndCommandBuffer(_commandBuffers.guiCommandBuffers[imageIndex]);
}



//--------------------------------------------
// Initialize the GUI vulkan backend
//
void VulkanModelViewer::initGuiBackend() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); //(void)io; //Don't know what this row is for so far
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(m_window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_instance;
	init_info.PhysicalDevice = m_physicalDevice;
	init_info.Device = m_device;
	init_info.QueueFamily = m_queueFamilyIndices.graphicsFamily.value();
	init_info.Queue = m_graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = _descriptorPools.guiDescriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = m_swapchainImages.size();
	init_info.ImageCount = m_swapchainImages.size();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	//init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, _renderPasses.guiRenderPass);

	// Upload Fonts
	m_commandUtil.setPoolInfo(_commandPool);
	VkCommandBuffer cmdbuf = m_commandUtil.beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(cmdbuf);
	m_commandUtil.endSingleTimeCommands(cmdbuf, m_graphicsQueue);
}



/**
* Drawing calls
*/

//--------------------------------------------------------------------------------------------------
// Draw a frame with gui
//
void VulkanModelViewer::drawFrame() {
	vkWaitForFences(m_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		_swapchainRebuild = true;
		return;
	}
	
	beginGuiRenderPass(imageIndex);
	updateUniformBuffer(imageIndex);

	// Check if a previous frame is using this image (i.e. there is its fence to wait on)
	if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(m_device, 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}
	// Mark the image as now being in use by this frame
	_imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];

	//Submit command buffers
	std::vector<VkCommandBuffer> submitCommandBuffers = getDrawSubmitCommandBuffers(imageIndex);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
	submitInfo.pCommandBuffers = submitCommandBuffers.data();
	VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_device, 1, &_inFlightFences[_currentFrame]);
	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//Present image
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		_swapchainRebuild = true;
		return;
	}

	//Step current frame
	_currentFrame = (_currentFrame + 1) % _maxFramesInFlight;
}

//--------------------------------------------------------------------------------------------------
// Get the command buffers to submit for this draw call
//
std::vector<VkCommandBuffer> VulkanModelViewer::getDrawSubmitCommandBuffers(uint32_t imageIndex) {
	std::vector<VkCommandBuffer> submitCommandBuffers{};
	if (_shadowOption == SHADOW_MAPPING)
		submitCommandBuffers.push_back(_commandBuffers.shadowCommandBuffers[imageIndex]);

	if (_shaderOption == DEFAULT)
		submitCommandBuffers.push_back(_commandBuffers.defaultCommandBuffers[imageIndex]);
	else if (_shaderOption == SCENE && _shadowOption == NO_SHADOW)
		submitCommandBuffers.push_back(_commandBuffers.sceneNoShadowCommandBuffers[imageIndex]);
	else if (_shaderOption == SCENE && _shadowOption == SHADOW_MAPPING)
		submitCommandBuffers.push_back(_commandBuffers.sceneCommandBuffers[imageIndex]);
	else if (_shaderOption == WIREFRAME_HOLLOW) {
		submitCommandBuffers.push_back(_commandBuffers.defaultCommandBuffers[imageIndex]);
		submitCommandBuffers.push_back(_commandBuffers.wireframeCommandBuffers[imageIndex]);
	}
	else if (_shaderOption == WIREFRAME_SOLID && _shadowOption == NO_SHADOW) {
		submitCommandBuffers.push_back(_commandBuffers.sceneNoShadowBlankModelCommandBuffers[imageIndex]);
		submitCommandBuffers.push_back(_commandBuffers.wireframeCommandBuffers[imageIndex]);
	}
	else if (_shaderOption == WIREFRAME_SOLID && _shadowOption == SHADOW_MAPPING) {
		submitCommandBuffers.push_back(_commandBuffers.sceneBlankModelCommandBuffers[imageIndex]);
		submitCommandBuffers.push_back(_commandBuffers.wireframeCommandBuffers[imageIndex]);
	}
		

	submitCommandBuffers.push_back(_commandBuffers.guiCommandBuffers[imageIndex]);
		
	return submitCommandBuffers;
}

//--------------------------------------------------------------------------------------------------
// Update the uniform buffers
//
void VulkanModelViewer::updateUniformBuffer(uint32_t currentImage) {
	//Initialize time
	static auto startTime = std::chrono::high_resolution_clock::now();
	static auto lastTime = std::chrono::high_resolution_clock::now();
	//Calculate time measures
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	float timeElapse = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();

	float minTimeElapse = 1 / _maxFrameRate;
	while (timeElapse < minTimeElapse) {
		currentTime = std::chrono::high_resolution_clock::now();
		timeElapse = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	}
	lastTime = currentTime;

	updateSceneInfo(timeElapse);

	//Fill ubo
	CameraInfoUBO cameraInfo{};
	cameraInfo.model = _repositionMatrix;
	cameraInfo.view = glm::lookAt(_camera.pos, _camera.pos + _camera.lookDir, _camera.upDir);;
	cameraInfo.proj = glm::perspective(glm::radians(60.0f), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, _initialDis * 10);
	cameraInfo.proj[1][1] *= -1;
	cameraInfo.cameraPos = _camera.pos;

	LightInfoUBO lightInfo{};
	lightInfo.lightColor = _lightSource.color;
	lightInfo.lightPos = _lightSource.pos;
	glm::mat4 lightModel = glm::mat4(1.0f);
	glm::mat4 lightView = glm::lookAt(_lightSource.pos, _modelCenter, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 lightProj = glm::perspective(glm::radians(60.0f), m_shadowMapExtent.width / (float)m_shadowMapExtent.height, 0.1f, _initialDis * 100);
	glm::mat4 lightMvp = lightProj * lightView * lightModel;
	lightInfo.lightMvp = lightMvp;

	void* data;
	vkMapMemory(m_device, _uniformBuffers.cameraUniformBuffers[currentImage].bufferMemory, 0, sizeof(cameraInfo), 0, &data);
	memcpy(data, &cameraInfo, sizeof(cameraInfo));
	vkUnmapMemory(m_device, _uniformBuffers.cameraUniformBuffers[currentImage].bufferMemory);

	vkMapMemory(m_device, _uniformBuffers.lightUniformBuffers[currentImage].bufferMemory, 0, sizeof(lightInfo), 0, &data);
	memcpy(data, &lightInfo, sizeof(lightInfo));
	vkUnmapMemory(m_device, _uniformBuffers.lightUniformBuffers[currentImage].bufferMemory);
}

//--------------------------------------------------------------------------------------------------
// Update the uniform buffers
//
void VulkanModelViewer::updateSceneInfo(float timeElapse) {
	_frameRate = 1 / timeElapse;

	_lightSource.pos = glm::rotate(glm::mat4(1.0f), _lightAngle * PI / 180.0f, _unitVec3Z) * glm::vec4(_lightDis, 0.0f, 0.0f, 1.0f);
	_lightSource.color = _lightDensity * glm::vec3(1.0f, 1.0f, 1.0f);

	glm::mat4 viewMat = glm::lookAt(_camera.pos, _camera.pos + _camera.lookDir, _camera.upDir);
	glm::vec3 modelCenterViewSpace = viewMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 viewMatInv = glm::inverse(viewMat);
	
	glm::vec3 rotateAxisX = glm::normalize(viewMatInv * glm::vec4(modelCenterViewSpace + _unitVec3X, 1.0f));
	glm::vec3 rotateAxisY = glm::normalize(viewMatInv * glm::vec4(modelCenterViewSpace + _unitVec3Y, 1.0f));
	glm::vec3 rotateAxisZ = glm::normalize(viewMatInv * glm::vec4(modelCenterViewSpace + _unitVec3Z, 1.0f));

	glm::mat4 rotationMat = glm::rotate(glm::mat4(1.0f), timeElapse * _rotationAngleRadiansX * PI / 180.0f, rotateAxisX);	
	rotationMat *= glm::rotate(glm::mat4(1.0f), timeElapse * _rotationAngleRadiansY * PI / 180.0f, rotateAxisY);
	rotationMat *= glm::rotate(glm::mat4(1.0f), timeElapse * _rotationAngleRadiansZ * PI / 180.0f, rotateAxisZ);
	rotationMat *= glm::rotate(glm::mat4(1.0f), _rotationAngleRadiansMouseX * PI / 180.0f, rotateAxisX);
	rotationMat *= glm::rotate(glm::mat4(1.0f), _rotationAngleRadiansMouseZ * PI / 180.0f, rotateAxisZ);

	_camera.pos = rotationMat * glm::vec4(_camera.pos, 1.0f) ;
	_camera.lookDir = rotationMat * glm::vec4(_camera.lookDir, 1.0f);
	_camera.upDir = rotationMat * glm::vec4(_camera.upDir, 1.0f);

	_camera.pos += _camera.lookDir * _translateZ * timeElapse;
	_camera.pos += _camera.lookDir * _translateMouseZ;

	_rotationAngleRadiansX = 0.0f;
	_rotationAngleRadiansY = 0.0f;
	_rotationAngleRadiansZ = 0.0f;

	_translateX = 0.0f;
	_translateY = 0.0f;
	_translateZ = 0.0f;

	_rotationAngleRadiansMouseX = 0.0f;
	_rotationAngleRadiansMouseZ = 0.0f;
	_translateMouseZ = 0.0f;
}



/**
* Cleanup calls for vulkan backend
*/

//--------------------------------------------
// Clean up all application resources
//
void VulkanModelViewer::cleanupVulkanBackend() {
	cleanupSwapchain();
	cleanupOffscreenRenderingResources();
	
	cleanupGuiVulkanBackend();
	destroySamplers();
	destroyDescriptorSetLayouts();
	destroySceneResources();

	for (size_t i = 0; i < _maxFramesInFlight; i++) {
		vkDestroySemaphore(m_device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, _imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device, _inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(m_device, _commandPool, nullptr);

	vkimpl::VulkanDebugUtil::DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

//--------------------------------------------------------------------------------------------------
// Clean up resources related to the swapchain
//
void VulkanModelViewer::cleanupSwapchain() {
	destroyPresentFramebuffers();
	destroyPresentImageResources();
	destroyPresentCommandBuffers();
	destroyPresentPipelines();
	destroyPresentRenderPasses();
	destroyPresentUniformBuffers();
	destroyPresentDescriptorPools();

	for (auto imageView : m_swapchainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	
}

//--------------------------------------------------------------------------------------------------
// Clean up image resources used for present
//
void VulkanModelViewer::destroyPresentImageResources() {
	destroyImageResource(_imageResources.sceneColor);
	destroyImageResource(_imageResources.sceneDepth);
}

//--------------------------------------------------------------------------------------------------
// Clean up framebuffers used for present
//
void VulkanModelViewer::destroyPresentFramebuffers() {
	for (int i = 0; i < m_swapchainImageNum; i++) {
		vkDestroyFramebuffer(m_device, _sceneFramebuffers[i], nullptr);
		vkDestroyFramebuffer(m_device, _guiFramebuffers[i], nullptr);
	}
}

//--------------------------------------------------------------------------------------------------
// Clean up command buffers used for present
//
void VulkanModelViewer::destroyPresentCommandBuffers() {
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.defaultCommandBuffers.size()), _commandBuffers.defaultCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.sceneCommandBuffers.size()), _commandBuffers.sceneCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.sceneCommandBuffers.size()), _commandBuffers.sceneNoShadowCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.sceneCommandBuffers.size()), _commandBuffers.sceneBlankModelCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.sceneCommandBuffers.size()), _commandBuffers.sceneNoShadowBlankModelCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.wireframeCommandBuffers.size()), _commandBuffers.wireframeCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.guiCommandBuffers.size()), _commandBuffers.guiCommandBuffers.data());
	vkFreeCommandBuffers(m_device, _commandPool, static_cast<uint32_t>(_commandBuffers.shadowCommandBuffers.size()), _commandBuffers.shadowCommandBuffers.data());
}

//--------------------------------------------------------------------------------------------------
// Clean up pipelines used for present
//
void VulkanModelViewer::destroyPresentPipelines() {
	vkDestroyPipeline(m_device, _pipelines.scenePipeline, nullptr);
	vkDestroyPipelineLayout(m_device, _pipelineLayouts.scenePipelineLayout, nullptr);

	vkDestroyPipeline(m_device, _pipelines.sceneNoLightingPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, _pipelineLayouts.sceneNoLightingPipelineLayout, nullptr);

	vkDestroyPipeline(m_device, _pipelines.wireframePipeline, nullptr);
	vkDestroyPipelineLayout(m_device, _pipelineLayouts.wireframePipelineLayout, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up render passes used for present
//
void VulkanModelViewer::destroyPresentRenderPasses() {
	vkDestroyRenderPass(m_device, _renderPasses.sceneRenderPass, nullptr);
	vkDestroyRenderPass(m_device, _renderPasses.wireframeRenderPass, nullptr);
	vkDestroyRenderPass(m_device, _renderPasses.guiRenderPass, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up uniform buffers used for present
//
void VulkanModelViewer::destroyPresentUniformBuffers() {
	destroyBufferResources(_uniformBuffers.cameraUniformBuffers);
	destroyBufferResources(_uniformBuffers.lightUniformBuffers);
}

//--------------------------------------------------------------------------------------------------
// Clean up descriptor pools used for present
//
void VulkanModelViewer::destroyPresentDescriptorPools() {
	vkDestroyDescriptorPool(m_device, _descriptorPools.sceneDescriptorPool, nullptr);
	vkDestroyDescriptorPool(m_device, _descriptorPools.cameraDescriptorPool, nullptr);
	vkDestroyDescriptorPool(m_device, _descriptorPools.lightDescriptorPool, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up resources for offscreen rendering
//
void VulkanModelViewer::cleanupOffscreenRenderingResources() {
	destroyOffscreenImageResources();
	destroyOffscreenFramebuffers();
	destroyOffscreenCommandBuffers();
	destroyOffscreenPipelines();
	destroyOffscreenRenderPasses();
	destroyOffscreenUniformBuffers();
	destroyOffscreenDescriptorPools();
}

//--------------------------------------------------------------------------------------------------
// Clean up offscreen image resources
//
void VulkanModelViewer::destroyOffscreenImageResources() {
	destroyImageResource(_imageResources.shadowDepth);
	destroyImageResource(_imageResources.defaultShadowDepth);
	for (auto textureImasgeResource : _textureResources)
		destroyImageResource(textureImasgeResource);
}

//--------------------------------------------------------------------------------------------------
// Clean up framebuffers used for present
//
void VulkanModelViewer::destroyOffscreenFramebuffers() {
	for (int i = 0; i < m_swapchainImageNum; i++) {
		vkDestroyFramebuffer(m_device, _shadowFramebuffers[i], nullptr);
	}
}

//--------------------------------------------------------------------------------------------------
// Clean up command buffers used for offscreen rendering
//
void VulkanModelViewer::destroyOffscreenCommandBuffers() {

}

//--------------------------------------------------------------------------------------------------
// Clean up pipelines used for offscreen rendering
//
void VulkanModelViewer::destroyOffscreenPipelines() {
	vkDestroyPipeline(m_device, _pipelines.shadowPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, _pipelineLayouts.shadowPipelineLayout, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up render passes used for offscreen rendering
//
void VulkanModelViewer::destroyOffscreenRenderPasses() {
	vkDestroyRenderPass(m_device, _renderPasses.shadowRenderPass, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up uniform buffers used for present
//
void VulkanModelViewer::destroyOffscreenUniformBuffers() {
	destroyBufferResources(_uniformBuffers.materialUniformBuffers);
}

//--------------------------------------------------------------------------------------------------
// Clean up descriptor pools used for offscreen rendering
//
void VulkanModelViewer::destroyOffscreenDescriptorPools() {
	vkDestroyDescriptorPool(m_device, _descriptorPools.guiDescriptorPool, nullptr);
	vkDestroyDescriptorPool(m_device, _descriptorPools.materialDescriptorPool, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up vulkan backend for gui
//
void VulkanModelViewer::cleanupGuiVulkanBackend() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

//--------------------------------------------------------------------------------------------------
// Clean up samplers
//
void VulkanModelViewer::destroySamplers() {
	vkDestroySampler(m_device, _samplers.textureSampler, nullptr);
	vkDestroySampler(m_device, _samplers.shadowSampler, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Desctroy descriptor set layouts of the application
//
void VulkanModelViewer::destroyDescriptorSetLayouts() {
	vkDestroyDescriptorSetLayout(m_device, _descriptorSetLayouts.sceneDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device, _descriptorSetLayouts.cameraDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device, _descriptorSetLayouts.lightDescriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_device, _descriptorSetLayouts.materialDescriptorSetLayout, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Desctroy vertex and index buffers
//
void VulkanModelViewer::destroySceneResources() {
	destroyModelBuffers();
}

//--------------------------------------------------------------------------------------------------
// Desctroy vertex and index buffers
//
void VulkanModelViewer::destroyModelBuffers() {
	vkDestroyBuffer(m_device, _vertexBuffer, nullptr);
	vkFreeMemory(m_device, _vertexBufferMemory, nullptr);
	vkDestroyBuffer(m_device, _indexBuffer, nullptr);
	vkFreeMemory(m_device, _indexBufferMemory, nullptr);
}



//--------------------------------------------------------------------------------------------------
// Clean up resources of images
//
void VulkanModelViewer::destroyImageResource(ImageResource image) {
	vkDestroyImageView(m_device, image.imageView, nullptr);
	vkDestroyImage(m_device, image.image, nullptr);
	vkFreeMemory(m_device, image.imageMemory, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Clean up vector of resources of buffers
//
void VulkanModelViewer::destroyBufferResources(std::vector<BufferResource> bufferResources) {
	for (int i = 0; i < bufferResources.size(); i++)
		destroyBufferResource(bufferResources[i]);
}

//--------------------------------------------------------------------------------------------------
// Clean up resources of buffers
//
void VulkanModelViewer::destroyBufferResource(BufferResource bufferResource) {
	vkDestroyBuffer(m_device, bufferResource.buffer, nullptr);
	vkFreeMemory(m_device, bufferResource.bufferMemory, nullptr);
}

//--------------------------------------------------------------------------------------------------
// Recreate the swapchain and related resources
//
void VulkanModelViewer::recreateSwapchain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_device);

	cleanupSwapchain();

	createSwapchain();
	createPresentDescriptorPools();
	createPresentUniformBuffers();
	createPresentRenderPasses();
	createPresentPipelines();
	createPresentCommandBuffers();
	createPresentImageResources();
	createPresentFramebuffers();
	createPresentDescriptorSets();
	
	beginPresentRenderPasses();

	_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);
	_swapchainRebuild = true;
}


/**
* Helpers
*/

//--------------------------------------------------------------------------------------------------
// Prepare VulkanImageCreateInfo for the creation of VkImage
//
vkimpl::VulkanImageInfo VulkanModelViewer::getImageInfo(ImageType imageType) {
	vkimpl::VulkanImageInfo imageInfo = {};

	imageInfo.extent.width = m_swapchainExtent.width;
	imageInfo.extent.height = m_swapchainExtent.height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

	if (imageType == COLOR_IMAGE || imageType == DEPTH_IMAGE) {
		imageInfo.numSamples = m_msaaSamples;
	}
	else if (imageType == TEXTURE_IMAGE) {
		imageInfo.numSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	if (imageType == COLOR_IMAGE) {
		imageInfo.format = m_swapchainImageFormat;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else if (imageType == DEPTH_IMAGE) {
		imageInfo.format = _defaultDepthFormat;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else if (imageType == TEXTURE_IMAGE) {
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	imageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	return imageInfo;
}

//--------------------------------------------------------------------------------------------------
// Create image, imageMemory and imageView from the image info
//
VulkanModelViewer::ImageResource  VulkanModelViewer::getImageResource(vkimpl::VulkanImageInfo imageInfo) {
	ImageResource attachment;
	m_imageUtil.setImageInfo(imageInfo);
	m_imageUtil.createImage(attachment.image, attachment.imageMemory);
	attachment.imageView = m_imageUtil.createImageView(attachment.image);
	return attachment;
}



/**
* GUI settings
*/


//--------------------------------------------------------------------------------------------------
// Set gui components we want to show
//
void VulkanModelViewer::setGuiComponents() {
	if (_swapchainRebuild) {
		ImGui_ImplVulkan_SetMinImageCount(MAX_FRAMES_IN_FLIGHT);
		_swapchainRebuild = false;
	}

	//Setup
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//Draw components
	
	//Tool window
	ImGui::Begin("Tools");
	//Set ImGui Componenets
	static bool selectObj{ false };
	static bool selectTex{ false };
	// open file dialog when user clicks this button
	if (ImGui::Button("Choose model obj")) {
		_fileDialog.Open();
		selectObj = true;
	}
	_fileDialog.Display();
	if (_fileDialog.HasSelected())
	{
		std::string selectedPath = _fileDialog.GetSelected().string();
		std::cout << "Selected filename: " << selectedPath << std::endl;
		if (selectObj && selectedPath != _modelPath) {
			_modelPath = selectedPath;
			_modelUpdated = true;
		}
		_fileDialog.ClearSelected();
		selectObj = false;
		selectTex = false;
	}

	//Light settings
	ImGui::SliderFloat("Light angle", &_lightAngle, 0.f, 360.f);
	ImGui::SliderFloat("Light Density", &_lightDensity, 0.f, 4.f);
	ImGui::SliderFloat("Light Distance", &_lightDis, 0.f, _initialDis * 10);
	ImGui::SliderFloat("Maximum FPS", &_maxFrameRate, 0.f, 1000.0f);

	//Shadow options
	const char* shadowOptions[2] = {"no shadow", "shadow mapping"};
	ImGui::ListBox("Shadow options", &_shadowOption, shadowOptions, 2);
	const char* shaderOptions[4] = { "default", "scene", "wireframe_hollow", "wireframe_solid"};
	ImGui::ListBox("Shader options", &_shaderOption, shaderOptions, 4);
	ImGui::End();

	//Information window
	ImGui::Begin("Information");
	ImGui::Text("Model path: %s", _modelPath.c_str());
	ImGui::Text("Model center: (%.4f, %.4f, %.4f)", _modelCenterViewSpace.x, _modelCenterViewSpace.y, _modelCenterViewSpace.z);
	ImGui::Text("Camera position: (%.4f, %.4f, %.4f)", _camera.pos.x, _camera.pos.y, _camera.pos.z);
	ImGui::Text("Camera look dir: (%.4f, %.4f, %.4f)", _camera.lookDir.x, _camera.lookDir.y, _camera.lookDir.z);
	ImGui::Text("Light source: (%.4f, %.4f, %.4f)", _lightSource.pos.x, _lightSource.pos.y, _lightSource.pos.z);
	ImGui::Text("FPS: %.2f", _frameRate);
	ImGui::End();

	//Render call
	ImGui::Render();
}



/**
* Application layer
*/

//--------------------------------------------------------------------------------------------------
// General update function called before each frame 
//
void VulkanModelViewer::update() {
	if (_modelUpdated) {
		updateModel();
		_modelUpdated = false;
	}
}

//--------------------------------------------------------------------------------------------------
// Function to update the model in our scene
//
void VulkanModelViewer::updateModel() {
	clearCurrentModel();

	loadOBJModel(_modelPath);
	createModelBuffer();
	beginObjectRenderPasses();
	_shaderOption = SCENE;

	updateModelInfo();
}

//--------------------------------------------------------------------------------------------------
// Update the information of the model
//
void VulkanModelViewer::updateModelInfo() {
	glm::vec3 ub{ -INFINITY, -INFINITY , -INFINITY };
	glm::vec3 lb{ INFINITY, INFINITY , INFINITY };
	_modelCenterOfGravity = { 0.0f, 0.0f, 0.0f };
	int counter{ 0 };
	for (Vertex vertex : _vertices) {
		ub = glm::max(vertex.pos, ub);
		lb = glm::min(vertex.pos, lb);
		if (counter > 0)
			_modelCenterOfGravity = _modelCenterOfGravity / float(counter + 1) * float(counter) + vertex.pos / float(counter + 1);
		else
			_modelCenterOfGravity = vertex.pos;
		counter++;
	}
	_modelCenter = (ub + lb) / 2.f;
	ub = ub - _modelCenter;
	float dis = std::max(ub[0], ub[1]);
	dis = std::max(dis, ub[2]);
	dis = dis * 2;

	_initialDis = dis;
	_lightDis = dis;
	_repositionMatrix = glm::translate(glm::mat4(1.0f), -_modelCenter);
	_camera.pos = glm::vec3(0.0f, 0.0f, dis);
	_camera.lookDir = glm::normalize(-_camera.pos);
	_camera.upDir = glm::vec3(0.0f, 1.0f, 0.0f);
	_keySensitivityTranslate = 1.0f * dis / 1.5;
	_mouseWheelSensitivityTranslate = 0.3f * dis / 1.5;
}

//--------------------------------------------------------------------------------------------------
// Function to clear the current model
//
void VulkanModelViewer::clearCurrentModel() {
	vkDeviceWaitIdle(m_device);
	_vertices.clear();
	_indices.clear();
	_shapes.clear();

	_materialCache = { _materialCache[0] };
	for (int i = 1; i < _uniformBuffers.materialUniformBuffers.size(); i++)
		destroyBufferResource(_uniformBuffers.materialUniformBuffers[i]);
	_uniformBuffers.materialUniformBuffers = { _uniformBuffers.materialUniformBuffers[0] };
	_descriptorSets.materialDescriptorSets = { _descriptorSets.materialDescriptorSets[0] };

	_texturePaths = { _texturePaths[0] };
	for (int i = 1; i < _textureResources.size(); i++)
		destroyImageResource(_textureResources[i]);
	_textureResources = { _textureResources[0] };
	destroyModelBuffers();
}

//--------------------------------------------------------------------------------------------------
// Handle inputs
//
void VulkanModelViewer::handleInput() {
	ImGuiIO& io = ImGui::GetIO();

	//Rotation handling
	//Keyboard rotation
	if (ImGui::IsKeyDown(GLFW_KEY_W)) {
		_rotationAngleRadiansX += _keySensitivityRotate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_S)) {
		_rotationAngleRadiansX -= _keySensitivityRotate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_A)) {
		_rotationAngleRadiansZ -= _keySensitivityRotate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_D)) {
		_rotationAngleRadiansZ += _keySensitivityRotate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_Q)) {
		_rotationAngleRadiansY += _keySensitivityRotate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_E)) {
		_rotationAngleRadiansY -= _keySensitivityRotate;
	}

	//Mouse button rotation
	glm::vec2 mosuePosCur(io.MousePos.x, io.MousePos.y);
	if (io.MouseDown[0]) {
		_rotationAngleRadiansMouseX -= _mouseDragSensitivityRotate * (mosuePosCur.y - _mousePivotPos.y);
		_rotationAngleRadiansMouseZ += _mouseDragSensitivityRotate * (mosuePosCur.x - _mousePivotPos.x);
	}
	_mousePivotPos = mosuePosCur;


	//Translation handling
	//Keyboard translation
	if (ImGui::IsKeyDown(GLFW_KEY_UP)) {
		_translateY += _keySensitivityTranslate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_DOWN)) {
		_translateY -= _keySensitivityTranslate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_LEFT)) {
		_translateX -= _keySensitivityTranslate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_RIGHT)) {
		_translateX += _keySensitivityTranslate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_PAGE_UP)) {
		_translateZ -= _keySensitivityTranslate;
	}
	else if (ImGui::IsKeyDown(GLFW_KEY_PAGE_DOWN)) {
		_translateZ += _keySensitivityTranslate;
	}
	
	//Mouse wheel translation
	_translateMouseZ += io.MouseWheel * _mouseWheelSensitivityTranslate;

	
}

/**
* Application layer helpers
*/

//--------------------------------------------------------------------------------------------------
// Load the vertices and indices from a .obj file using tiny_obj_loader
//
void VulkanModelViewer::loadOBJModel(std::string path) {


	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	std::string directory;
	const size_t last_slash_idx = path.rfind('\\');
	if (std::string::npos != last_slash_idx)
	{
		directory = path.substr(0, last_slash_idx);
	}

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), directory.c_str())) {
		throw std::runtime_error(warn + err);
	}
	std::cout << warn + err << std::endl;

	//Process materials
	std::map<int, int> materialIndexMap;
	
	//Preallocation of memory
	int ind_count = 0;
	for (const auto& shape : shapes)
		ind_count += shape.mesh.indices.size();
	_indices.reserve(ind_count);
	_vertices.reserve(ind_count / 3 + 1);

	//Process shapes
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (const auto& shape : shapes) {
		if (shape.mesh.indices.size() <= 0)
			continue;
		Shape currentShape{};
		currentShape.indexBase = _indices.size();
		std::map<int, std::vector<uint32_t>> matGroupIndMap{};
		int face = 0;
		int num_face_vertices = shape.mesh.num_face_vertices[face];
		int materialIdLocal = shape.mesh.material_ids[face];
		int vertexCount = 0;
		for (const auto& index : shape.mesh.indices) {
			//Update face information
			if (vertexCount == num_face_vertices) {
				face++;
				num_face_vertices = shape.mesh.num_face_vertices[face];
				materialIdLocal = shape.mesh.material_ids[face];
				vertexCount = 0;
			}

			Vertex vertex{};

			//Populate per vertex information
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			if (index.texcoord_index >= 0 && attrib.texcoords.size() > 0)
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			else
				vertex.texCoord = { 0.0f, 0.0f };

			if (attrib.colors.size() > 0)
				vertex.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]
				};
			else
				vertex.color = { 0.0f, 0.0f, 0.0f };

			if (index.normal_index >= 0 && attrib.normals.size() > 0)
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
			};
			else
				vertex.normal = { 0.0f, 0.0f, 0.0f };

			//Populate material information
			if (materialIndexMap.find(materialIdLocal) == materialIndexMap.end()) {
				if (materialIdLocal >= 0) {
					Material mat = loadMaterial(directory, materials[materialIdLocal]);
					auto matIt = std::find(_materialCache.begin(), _materialCache.end(), mat);
					materialIndexMap.insert({ materialIdLocal, matIt - _materialCache.begin() });
					if (matIt == _materialCache.end()) {
						_materialCache.push_back(mat);
						_uniformBuffers.materialUniformBuffers.push_back(getMaterialUniformBuffer(mat));
						_descriptorSets.materialDescriptorSets.push_back(getMaterialDescriptorSet(mat, _uniformBuffers.materialUniformBuffers.back().buffer));
					}
				}
				else {
					materialIndexMap.insert({ materialIdLocal, 0 });
				}
				
			}
			vertex.materialId = materialIndexMap[materialIdLocal];

			//Count vertex
			vertexCount++;

			//Check the uniqueness of the vertex id and update the unique vertex map
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(_vertices.size());
				_vertices.push_back(vertex);
			}

			//Save the index in the material group
			if (matGroupIndMap.find(vertex.materialId) == matGroupIndMap.end())
				matGroupIndMap[vertex.materialId] = { uniqueVertices[vertex] };
			else
				matGroupIndMap[vertex.materialId].push_back(uniqueVertices[vertex]);
		}

		//Build the material group in the shape and insert its indices to the indices array
		for (auto matGroupInds : matGroupIndMap) {
			MaterialGroup currenMaterialGroup {_indices.size(), matGroupInds.second.size(), matGroupInds.first};
			currentShape.materialGroups.push_back(currenMaterialGroup);
			_indices.insert(_indices.end(), matGroupInds.second.begin(), matGroupInds.second.end());
		}

		currentShape.indexCount = _indices.size() - currentShape.indexBase;
		_shapes.push_back(currentShape);
	}
}

//--------------------------------------------------------------------------------------------------
// Load the material into cache from the material defined in .mtl file
//
VulkanModelViewer::Material VulkanModelViewer::loadMaterial(std::string directory, tinyobj::material_t material) {
	Material mat{};

	//Material parameter settings
	mat.ambient = { material.ambient[0], material.ambient[1], material.ambient[2] };
	mat.diffuse = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
	mat.specular = { material.specular[0], material.specular[1], material.specular[2] };
	mat.transmittance = { material.transmittance[0], material.transmittance[1], material.transmittance[2] };
	mat.emission = { material.emission[0], material.emission[1], material.emission[2] };
	mat.shininess = material.shininess;
	mat.ior = material.ior;
	mat.dissolve = material.dissolve;
	mat.illumModelIndex = material.illum;
	mat.roughness = material.roughness;
	mat.metallic = material.metallic;
	mat.sheen = material.sheen;
	mat.clearcoat_thickness = material.clearcoat_thickness;
	mat.clearcoat_roughness = material.clearcoat_roughness;
	mat.anisotropy = material.anisotropy;
	mat.anisotropy_rotation = material.anisotropy_rotation;

	//Texuture settings
	mat.ambient_texture_ind = loadTexture(directory, material.ambient_texname);
	mat.diffuse_texture_ind = loadTexture(directory, material.diffuse_texname);
	mat.specular_texture_ind = loadTexture(directory, material.specular_texname);
	mat.specular_highlight_texture_ind = loadTexture(directory, material.specular_highlight_texname);
	mat.bump_texture_ind = loadTexture(directory, material.bump_texname);
	mat.displacement_texture_ind = loadTexture(directory, material.displacement_texname);
	mat.alpha_texture_ind = loadTexture(directory, material.alpha_texname);
	mat.reflection_texture_ind = loadTexture(directory, material.reflection_texname);
	mat.roughness_texture_ind = loadTexture(directory, material.roughness_texname);
	mat.metallic_texture_ind = loadTexture(directory, material.metallic_texname);
	mat.sheen_texture_ind = loadTexture(directory, material.sheen_texname);
	mat.emissive_texture_ind = loadTexture(directory, material.emissive_texname);
	mat.normal_texture_ind = loadTexture(directory, material.normal_texname);

	updateMaterialUbo(mat);

	return mat;
}


//--------------------------------------------------------------------------------------------------
// Update the ubo according to the material information
//
void VulkanModelViewer::updateMaterialUbo(Material& mat) {
	mat.ubo.ambient = mat.ambient;
	mat.ubo.diffuse = mat.diffuse;
	mat.ubo.specular = mat.specular;
	mat.ubo.transmittance = mat.transmittance;
	mat.ubo.emission = mat.emission;
	mat.ubo.shininess = mat.shininess;
	mat.ubo.ior = mat.ior;
	mat.ubo.dissolve = mat.dissolve;
	mat.ubo.illumModelIndex = mat.illumModelIndex;
	mat.ubo.roughness = mat.roughness;
	mat.ubo.metallic = mat.metallic;
	mat.ubo.sheen = mat.sheen;
	mat.ubo.clearcoat_thickness = mat.clearcoat_thickness;
	mat.ubo.clearcoat_roughness = mat.clearcoat_roughness;
	mat.ubo.anisotropy = mat.anisotropy;
	mat.ubo.anisotropy_rotation = mat.anisotropy_rotation;
	mat.ubo.ambient_texture_ind = mat.ambient_texture_ind;
	mat.ubo.diffuse_texture_ind = mat.diffuse_texture_ind;
	mat.ubo.specular_texture_ind = mat.specular_texture_ind;
	mat.ubo.specular_highlight_texture_ind = mat.specular_highlight_texture_ind;
	mat.ubo.bump_texture_ind = mat.bump_texture_ind;
	mat.ubo.displacement_texture_ind = mat.displacement_texture_ind;
	mat.ubo.alpha_texture_ind = mat.alpha_texture_ind;
	mat.ubo.reflection_texture_ind = mat.reflection_texture_ind;
	mat.ubo.roughness_texture_ind = mat.roughness_texture_ind;
	mat.ubo.metallic_texture_ind = mat.metallic_texture_ind;
	mat.ubo.sheen_texture_ind = mat.sheen_texture_ind;
	mat.ubo.emissive_texture_ind = mat.emissive_texture_ind;
	mat.ubo.normal_texture_ind = mat.normal_texture_ind;
}

//--------------------------------------------------------------------------------------------------
// Create the uniform buffer and copy data from material information
//
VulkanModelViewer::BufferResource VulkanModelViewer::getMaterialUniformBuffer(Material mat) {
	BufferResource uniformBuffer;
	m_bufferUtil.createBuffer(sizeof(MaterialUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer.buffer, uniformBuffer.bufferMemory);

	void* data;
	vkMapMemory(m_device, uniformBuffer.bufferMemory, 0, sizeof(mat.ubo), 0, &data);
	memcpy(data, &mat.ubo, sizeof(mat.ubo));
	vkUnmapMemory(m_device, uniformBuffer.bufferMemory);

	return uniformBuffer;
}

//--------------------------------------------------------------------------------------------------
// Create the descriptor set for given material
//
VkDescriptorSet VulkanModelViewer::getMaterialDescriptorSet(Material mat, VkBuffer matUniformBuffer){
	//Initialization
	VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
	_descriptorSetInfos.materialDescriptorInfo.bufferInfos.clear();
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.clear();

	//Uniform buffer
	VkDescriptorBufferInfo bufferInfo = { matUniformBuffer, 0, sizeof(MaterialUBO) };
	_descriptorSetInfos.materialDescriptorInfo.bufferInfos.push_back(bufferInfo);
	//Images
	VkDescriptorImageInfo imageInfo{};
	imageInfo = { _samplers.textureSampler, _textureResources[mat.ambient_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.diffuse_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.specular_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.specular_highlight_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.bump_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.displacement_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.alpha_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);
	imageInfo = { _samplers.textureSampler, _textureResources[mat.reflection_texture_ind].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	_descriptorSetInfos.materialDescriptorInfo.imageInfos.push_back(imageInfo);

	m_descriptorUtil.createDescriptorSet(_descriptorPools.materialDescriptorPool, _descriptorSetLayouts.materialDescriptorSetLayout, _descriptorSetInfos.materialDescriptorInfo, descriptorSet);

	return descriptorSet;
}

//--------------------------------------------------------------------------------------------------
// Return the index of the texture in the texture cache,
// and load the texture image if its path is not already in the cache.
// 
//
int VulkanModelViewer::loadTexture(std::string directory, std::string relativePath) {
	int ind;
	auto texPathBeginIt = _texturePaths.begin();
	auto texPathEndIt = _texturePaths.end();
	if (relativePath.size() > 0) {
		relativePath = preprocessPath(relativePath);
		std::string fullPath = directory + "\\" + relativePath;
		auto texPathFindIt = std::find(texPathBeginIt, texPathEndIt, fullPath);
		if (texPathFindIt == texPathEndIt) {
			_textureResources.push_back(createTextureImageResource(fullPath));
			_texturePaths.push_back(fullPath);
			ind = _textureResources.size() - 1;
		}
		else {
			ind = texPathFindIt - texPathBeginIt;
		}	
	}
	else {
		ind = 0;
	}
	return ind;
}

//--------------------------------------------------------------------------------------------------
// Scroll callback for glfw
//
void VulkanModelViewer::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

//--------------------------------------------------------------------------------------------------
// Mouse button callback for glfw
//
void VulkanModelViewer::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	ImGuiIO& io = ImGui::GetIO();
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
		io.MouseClicked[0] = true;
	else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
		io.MouseClicked[0] = false;
}

//--------------------------------------------------------------------------------------------------
// Cursor position callback for glfw
//
void VulkanModelViewer::glfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = { (float)xpos, (float)ypos };
}

/**
* General helpers
*/

//--------------------------------------------------------------------------------------------------
// Preprocess a file path string to unify its format
//
std::string VulkanModelViewer::preprocessPath(std::string path) {
	std::regex reg("\\\\\\\\");
	path = std::regex_replace(path.c_str(), reg, "\\");
	return path;
}
