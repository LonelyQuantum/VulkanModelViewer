#include "vulkan_app_base.h"

#include <imgui.h>
#define NOMINMAX
#include <imfilebrowser.h>

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <array>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <regex>
#include <thread>

#include "configFile.h"

//--------------------------------------------------------------------------------------------------
// Small rasterization OBJ model viewer
//
class VulkanModelViewer : public VulkanAppBase {
public:

	//Vertex
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
		int materialId;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions(5);

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, normal);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32_SINT;
			attributeDescriptions[4].offset = offsetof(Vertex, materialId);
			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord && materialId == other.materialId;
		}
	};

	void run();

	//App settings
	VkSampleCountFlagBits m_msaaSamples;
	int m_mipLevel;
	VkExtent2D m_shadowMapExtent;
	VkClearColorValue m_sceneClearColor;
	

private:
	//Type of the image under operation
	enum ImageType
	{
		COLOR_IMAGE = 0,
		DEPTH_IMAGE = 1,
		TEXTURE_IMAGE = 2
	};

	//Type of shader in use
	enum ShaderType {
		DEFAULT = 0,
		SCENE = 1,
		WIREFRAME_HOLLOW = 2,
		WIREFRAME_SOLID = 3
	};

	//Type of shadow in use
	enum ShadowType {
		NO_SHADOW = 0,
		SHADOW_MAPPING = 1
	};

	//App info structs
	struct Camera {
		glm::vec3 pos;
		glm::vec3 lookDir;
		glm::vec3 upDir;
	};

	struct PointLightSource {
		glm::vec3 pos;
		glm::vec3 color;
	};

	struct ImageResource {
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	};

	struct BufferResource {
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
	};

	// Uniform buffer structs
	struct CameraInfoUBO {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec3 cameraPos;
	};

	struct LightInfoUBO {
		alignas(16) glm::vec3 lightPos;
		alignas(16) glm::vec3 lightColor;
		alignas(16) glm::mat4 lightMvp;
	};

	struct MaterialUBO {
		alignas(16) glm::vec3 ambient;
		alignas(16) glm::vec3 diffuse;
		alignas(16) glm::vec3 specular;
		alignas(16) glm::vec3 transmittance;
		alignas(16) glm::vec3 emission;

		alignas(4) int illumModelIndex;

		alignas(4) float shininess;
		alignas(4) float ior;
		alignas(4) float dissolve;
		alignas(4) float roughness;          
		alignas(4) float metallic;           
		alignas(4) float sheen;              
		alignas(4) float clearcoat_thickness; 
		alignas(4) float clearcoat_roughness; 
		alignas(4) float anisotropy;         
		alignas(4) float anisotropy_rotation; 

		alignas(4) int ambient_texture_ind;
		alignas(4) int diffuse_texture_ind;
		alignas(4) int specular_texture_ind;
		alignas(4) int specular_highlight_texture_ind;
		alignas(4) int bump_texture_ind;
		alignas(4) int displacement_texture_ind;
		alignas(4) int alpha_texture_ind;
		alignas(4) int reflection_texture_ind;
		alignas(4) int roughness_texture_ind;
		alignas(4) int metallic_texture_ind;
		alignas(4) int sheen_texture_ind;
		alignas(4) int emissive_texture_ind;
		alignas(4) int normal_texture_ind;
	};

	//Material group
	struct MaterialGroup {
		int indexBase;
		int indexCount;
		int materialId;
	};

	//Shape
	struct Shape {
		int indexBase;
		int indexCount;
		std::vector<MaterialGroup> materialGroups;
	};

	//Material
	struct Material {
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec3 transmittance;
		glm::vec3 emission;

		float shininess;
		float ior;
		float dissolve;

		int illumModelIndex;

		int ambient_texture_ind{ 0 };             // map_Ka
		int diffuse_texture_ind{ 0 };             // map_Kd
		int specular_texture_ind{ 0 };            // map_Ks
		int specular_highlight_texture_ind{ 0 };  // map_Ns
		int bump_texture_ind{ 0 };                // map_bump, map_Bump, bump
		int displacement_texture_ind{ 0 };        // disp
		int alpha_texture_ind{ 0 };               // map_d
		int reflection_texture_ind{ 0 };          // refl

		// PBR extension
		// http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
		float roughness;            // [0, 1] default 0
		float metallic;             // [0, 1] default 0
		float sheen;                // [0, 1] default 0
		float clearcoat_thickness;  // [0, 1] default 0
		float clearcoat_roughness;  // [0, 1] default 0
		float anisotropy;           // aniso. [0, 1] default 0
		float anisotropy_rotation;  // anisor. [0, 1] default 0
		int roughness_texture_ind{ 0 };  // map_Pr
		int metallic_texture_ind{ 0 };   // map_Pm
		int sheen_texture_ind{ 0 };      // map_Ps
		int emissive_texture_ind{ 0 };   // map_Ke
		int normal_texture_ind{ 0 };     // norm. For normal mapping.

		MaterialUBO ubo{};

		bool operator==(const Material& other) const {
			return ambient == other.ambient && diffuse == other.diffuse && specular == other.specular && transmittance == other.transmittance
				&& emission == other.emission && shininess == other.shininess && ior == other.ior && dissolve == other.dissolve
				&& illumModelIndex == other.illumModelIndex && ambient_texture_ind == other.ambient_texture_ind 
				&& diffuse_texture_ind == other.diffuse_texture_ind && specular_texture_ind == other.specular_texture_ind 
				&& specular_highlight_texture_ind == other.specular_highlight_texture_ind && bump_texture_ind == other.bump_texture_ind 
				&& displacement_texture_ind == other.displacement_texture_ind && alpha_texture_ind == other.alpha_texture_ind
				&& reflection_texture_ind == other.reflection_texture_ind && roughness == other.roughness && metallic == other.metallic && sheen == other.sheen
				&& clearcoat_thickness == other.clearcoat_thickness && clearcoat_roughness == other.clearcoat_roughness && anisotropy == other.anisotropy
				&& anisotropy_rotation == other.anisotropy_rotation && roughness_texture_ind == other.roughness_texture_ind
				&& metallic_texture_ind == other.metallic_texture_ind && sheen_texture_ind == other.sheen_texture_ind 
				&& emissive_texture_ind == other.emissive_texture_ind && normal_texture_ind == other.normal_texture_ind;
		}
	};

	//Application
	void init();
	void mainLoop();
	void cleanup();
	void initAppSettings();
	void initAppResources();
	void initDefaultMaterial();

	//Control functions
	void setGuiComponents();

	//Vulkan backend
	void initVulkan();
	void initRenderSettings();

	void initRenderPasses();
	void createPresentRenderPasses();
	void createSceneRenderPass();
	void createWireframeRenderPass();
	void createShadowRenderPass();
	void createGuiRenderPass();

	void initImageResources();
	void createPresentImageResources();
	ImageResource createTextureImageResource(std::string texPath);

	void initSceneResources();
	void createModelBuffer();

	void initFramebuffers();
	void createPresentFramebuffers();
	void createSceneFramebuffers();
	void createGuiFramebuffers();
	void createShadowFramebuffers();
	void createFramebuffers(VkRenderPass renderPass, VkExtent2D extent, std::vector<VkImageView> imageViews, std::vector<std::vector<VkImageView>> imageViewArrays, std::vector<std::string> framebufferNames, std::vector<VkFramebuffer>& framebuffers);

	void initUniformBuffers();
	void createPresentUniformBuffers();
	void createUniformBuffers(VkDeviceSize bufferSize, std::vector<BufferResource>& uniformBuffers);

	void initDescriptorSetLayouts();
	void createSceneDescriptorSetLayout();
	void createCameraDescriptorSetLayout();
	void createLightDescriptorSetLayout();
	void createMaterialDescriptorSetLayout();

	void initDescriptorPools();
	void createPresentDescriptorPools();
	void createSceneDescriptorPool();
	void createCameraDescriptorPool();
	void createLightDescriptorPool();
	void createMaterialDescriptorPool();
	void createGuiDescriptorPool();

	void createSamplers();
	void createTextureSampler();
	void createShadowSampler();

	void initCommandPools();
	
	void initPipelines();
	void createPresentPipelines();
	void createScenePipeline();
	void createSceneNoLightingPipeline();
	void createWireframePipeline();
	void createShadowPipeline();

	void initDescriptorSets();
	void createPresentDescriptorSets();
	void createSceneDescriptorSets();
	void createNoShadowSceneDescriptorSets();
	void createCameraDescriptorSets();
	void createLightDescriptorSets();

	void initCommandBuffers();
	void createPresentCommandBuffers();
	void createDefaultCommandBuffers();
	void createShadowCommandBuffers();
	void createSceneCommandBuffers();
	void createSceneBlankModelCommandBuffers();
	void createNoShadowSceneCommandBuffers();
	void createNoShadowSceneBlankModelCommandBuffers();
	void createWireframeCommandBuffers();
	void createGuiCommandBuffers();

	void initSyncObjects();
	void createPresentSyncObjects();

	void beginRenderPasses();
	void beginPresentRenderPasses();
	void beginDefaultRenderPass();
	void beginObjectRenderPasses();
	void beginSceneRenderPass();
	void beginNoShadowSceneRenderPass();
	void beginSceneBlankModelRenderPass();
	void beginNoShadowSceneBlankModelRenderPass();
	void beginWireframeRenderPass();
	void beginShadowRenderPass();
	void beginGuiRenderPass(uint32_t imageIndex);

	void initGuiBackend();

	//Cleanup calls
	void cleanupVulkanBackend();
	void cleanupSwapchain();
	void destroyPresentImageResources();
	void destroyPresentFramebuffers();
	void destroyPresentCommandBuffers();
	void destroyPresentPipelines();
	void destroyPresentRenderPasses();
	void destroyPresentUniformBuffers();
	void destroyPresentDescriptorPools();
	void cleanupOffscreenRenderingResources();
	void destroyOffscreenImageResources();
	void destroyOffscreenFramebuffers();
	void destroyOffscreenCommandBuffers();
	void destroyOffscreenPipelines();
	void destroyOffscreenRenderPasses();
	void destroyOffscreenUniformBuffers();
	void destroyOffscreenDescriptorPools();
	void cleanupGuiVulkanBackend();
	void destroySamplers();
	void destroyDescriptorSetLayouts();
	void destroySceneResources();
	void destroyModelBuffers();
	void destroyImageResource(ImageResource imageResource);
	void destroyBufferResources(std::vector<BufferResource> bufferResources);
	void destroyBufferResource(BufferResource bufferResource);

	//Drawing calls
	void drawFrame();
	void recreateSwapchain();
	std::vector<VkCommandBuffer> VulkanModelViewer::getDrawSubmitCommandBuffers(uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);
	void updateSceneInfo(float timeElapse);

	//Vulkan backend helpers
	vkimpl::VulkanImageInfo getImageInfo(ImageType imageType);
	ImageResource getImageResource(vkimpl::VulkanImageInfo imageInfo);

	//Control Layer
	void update();
	void handleInput();
	void updateModel();
	void clearCurrentModel();
	void updateModelInfo();
	void loadOBJModel(std::string path);
	Material loadMaterial(std::string directory, tinyobj::material_t material);
	void updateMaterialUbo(Material& mat);
	int loadTexture(std::string directory, std::string relativePath);
	BufferResource getMaterialUniformBuffer(Material mat);
	VkDescriptorSet getMaterialDescriptorSet(Material mat, VkBuffer matUniformBuffer);
	static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void glfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

	//General helpers
	std::string preprocessPath(std::string path);
	
	//Settings
	VkFormat _defaultDepthFormat;

	//Vulkan render backend

	//render passes
	struct{
		VkRenderPass sceneRenderPass;
		VkRenderPass wireframeRenderPass;
		VkRenderPass shadowRenderPass;
		VkRenderPass guiRenderPass;
	} _renderPasses;

	//Image resources
	struct {
		ImageResource sceneColor;
		ImageResource sceneDepth;
		ImageResource shadowDepth;
		ImageResource defaultShadowDepth;
	} _imageResources;
	
	//Framebuffers
	std::vector<VkFramebuffer> _sceneFramebuffers;
	std::vector<VkFramebuffer> _guiFramebuffers;
	std::vector<VkFramebuffer> _shadowFramebuffers;

	//3D Resources
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	VkBuffer _vertexBuffer;
	VkDeviceMemory _vertexBufferMemory;
	VkBuffer _indexBuffer;
	VkDeviceMemory _indexBufferMemory;
	

	//Texture resources
	std::vector<std::string> _texturePaths;
	std::vector<ImageResource> _textureResources;

	//Scene informations and resources
	std::vector<Shape> _shapes;
	std::vector<Material> _materialCache;

	//Uniform buffers
	struct {
		std::vector<BufferResource> cameraUniformBuffers;
		std::vector<BufferResource> lightUniformBuffers;
		std::vector<BufferResource> materialUniformBuffers;
	} _uniformBuffers;;

	//Descriptor informations
	struct {
		vkimpl::DescriptorSetInfo sceneDescriptorInfo{};
		vkimpl::DescriptorSetInfo cameraDescriptorInfo{};
		vkimpl::DescriptorSetInfo lightDescriptorInfo{};
		vkimpl::DescriptorSetInfo materialDescriptorInfo{};
		vkimpl::DescriptorSetInfo guiDescriptorInfo{};
	} _descriptorSetInfos;

	//Descriptor layours
	struct {
		VkDescriptorSetLayout sceneDescriptorSetLayout;
		VkDescriptorSetLayout cameraDescriptorSetLayout;
		VkDescriptorSetLayout materialDescriptorSetLayout;
		VkDescriptorSetLayout lightDescriptorSetLayout;
		
	} _descriptorSetLayouts;

	//Descriptor pools
	struct {
		VkDescriptorPool sceneDescriptorPool;
		VkDescriptorPool cameraDescriptorPool;
		VkDescriptorPool lightDescriptorPool;
		VkDescriptorPool materialDescriptorPool;
		VkDescriptorPool guiDescriptorPool;
	} _descriptorPools;

	//Descriptor sets
	struct {
		std::vector<VkDescriptorSet> sceneDescriptorSets;
		std::vector<VkDescriptorSet> sceneNoShadowDescriptorSets;
		std::vector<VkDescriptorSet> cameraDescriptorSets;
		std::vector<VkDescriptorSet> lightDescriptorSets;
		std::vector<VkDescriptorSet> materialDescriptorSets;
	} _descriptorSets;

	//Samplers
	struct {
		VkSampler textureSampler;
		VkSampler shadowSampler;
	} _samplers;

	//Pipeline layouts and piplines
	struct {
		VkPipelineLayout scenePipelineLayout;
		VkPipelineLayout sceneNoLightingPipelineLayout;
		VkPipelineLayout wireframePipelineLayout;
		VkPipelineLayout shadowPipelineLayout;
	} _pipelineLayouts;

	struct {
		VkPipeline scenePipeline;
		VkPipeline sceneNoLightingPipeline;
		VkPipeline wireframePipeline;
		VkPipeline shadowPipeline;
	} _pipelines;
	

	//Command Pools
	VkCommandPool _commandPool;

	//Pipelines

	//Command buffers
	struct {
		std::vector<VkCommandBuffer> defaultCommandBuffers;
		std::vector<VkCommandBuffer> sceneCommandBuffers;
		std::vector<VkCommandBuffer> sceneNoShadowCommandBuffers;
		std::vector<VkCommandBuffer> sceneBlankModelCommandBuffers;
		std::vector<VkCommandBuffer> sceneNoShadowBlankModelCommandBuffers;
		std::vector<VkCommandBuffer> wireframeCommandBuffers;
		std::vector<VkCommandBuffer> shadowCommandBuffers;
		std::vector<VkCommandBuffer> guiCommandBuffers;
	} _commandBuffers;
	

	//Sync objects
	int _maxFramesInFlight;
	std::vector<VkSemaphore> _imageAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	std::vector<VkFence> _inFlightFences;
	std::vector<VkFence> _imagesInFlight;
	size_t _currentFrame = 0;

	//Draw control
	bool _swapchainRebuild;

	//Gui backend
	ImGui::FileBrowser _fileDialog{};

	//Control layer
	std::string _modelPath;
	bool _modelUpdated;

	int _shadowOption{ 0 };
	int _shaderOption{ 0 };

	//App info
	float _frameRate{ 0.0f };
	float _maxFrameRate = 120.0f;
	Camera _camera{};
	PointLightSource _lightSource{};

	glm::vec3 _modelCenter{ 0.f, 0.f, 0.f };
	glm::vec3 _modelCenterOfGravity{ 0.f, 0.f, 0.f };
	glm::vec3 _modelCenterViewSpace{ 0.f, 0.f, 0.f };
	glm::vec3 _modelCenterOfGravityViewSpace{ 0.f, 0.f, 0.f };

	glm::mat4 _viewMatrix = glm::mat4(1.0f);
	glm::mat4 _repositionMatrix = glm::mat4(1.0f);

	glm::vec3 _lightPosViewSpace{ 0.0f, 0.0f, 0.0f };
	float _lightAngle{ 45.f };
	float _lightDensity{ 2.f };
	float _lightDis{ 10.f };
	float _initialDis{ 10.f };

	float _keySensitivityRotate = 30.0f;
	float _keySensitivityTranslate = 2.5f;
	float _mouseWheelSensitivityTranslate = 2.5f;
	float _mouseDragSensitivityRotate = 0.5f;
	bool _mouseLeftKeyDown;
	glm::vec2 _mousePivotPos{ 0.0f, 0.0f };

	glm::mat4 _viewRotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 _viewTranslation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	float _rotationAngleRadiansX;
	float _rotationAngleRadiansY;
	float _rotationAngleRadiansZ;
	float _rotationAngleRadiansMouseX;
	float _rotationAngleRadiansMouseZ;

	float _translateX;
	float _translateY;
	float _translateZ;
	float _translateMouseZ;

	glm::vec3 _unitVec3X{ 1.0f, 0.0f, 0.0f };
	glm::vec3 _unitVec3Y{ 0.0f, 1.0f, 0.0f };
	glm::vec3 _unitVec3Z{ 0.0f, 0.0f, 1.0f };
};

namespace std {
	template<> struct hash<VulkanModelViewer::Vertex> {
		size_t operator()(VulkanModelViewer::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}