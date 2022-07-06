#ifndef VULKAN_COMMON_UTILS
#define  VULKAN_COMMON_UTILS
#include <vulkan/vulkan_core.h>
#include <vector>
#include <map>
#include <string>

namespace vkimpl
{
struct VulkanStructCommon  // Helper struct to link extensions together
{
	VkStructureType sType;
	void* pNext;
};

class VulkanCommonUtils {
public:
    // Physical device feature struct getters
    std::string getVkBool32StructName(void* pStructFeatures);
    std::vector<const char*> getVkBool32StructVector(void* pStructFeatures);
    std::map<const char*, int> getVkBool32StructMap(void* pStructFeatures);
    VkBool32 getVkBool32StructValue(void* pStructFeatures, const char* fieldName);
    std::vector<VkBool32> getVkBool32StructValues(void* pStructFeatures);

    VkBool32 getVkBool32StructValue(VkPhysicalDeviceFeatures vkStruct, const char* fieldName);
    VkBool32 getVkBool32StructValue(VkPhysicalDeviceFeatures2* pVkStruct, const char* fieldName);
    VkBool32 getVkBool32StructValue(VkPhysicalDeviceVulkan11Features* pVkStruct, const char* fieldName);
    VkBool32 getVkBool32StructValue(VkPhysicalDeviceVulkan12Features* pVkStruct, const char* fieldName);
    VkBool32 getVkBool32StructValue(VkPhysicalDeviceAccelerationStructureFeaturesKHR* pVkStruct, const char* fieldName);
    VkBool32 getVkBool32StructValue(VkPhysicalDeviceRayTracingPipelineFeaturesKHR* pVkStruct, const char* fieldName);
    std::vector<VkBool32> getVkBool32StructValues(VkPhysicalDeviceFeatures vkStruct);
    std::vector<VkBool32> getVkBool32StructValues(VkPhysicalDeviceFeatures2* pVkStruct);
    std::vector<VkBool32> getVkBool32StructValues(VkPhysicalDeviceVulkan11Features* pVkStruct);
    std::vector<VkBool32> getVkBool32StructValues(VkPhysicalDeviceVulkan12Features* pVkStruct);
    std::vector<VkBool32> getVkBool32StructValues(VkPhysicalDeviceAccelerationStructureFeaturesKHR* pVkStruct);
    std::vector<VkBool32> getVkBool32StructValues(VkPhysicalDeviceRayTracingPipelineFeaturesKHR* pVkStruct);

    std::vector<const char*> physicalDeviceFeatures2Vector{
        "robustBufferAccess", "fullDrawIndexUint32", "imageCubeArray",
        "independentBlend", "geometryShader", "tessellationShader",
        "sampleRateShading", "dualSrcBlend", "logicOp",
        "multiDrawIndirect", "drawIndirectFirstInstance", "depthClamp",
        "depthBiasClamp", "fillModeNonSolid", "depthBounds",
        "wideLines", "largePoints", "alphaToOne",
        "multiViewport", "samplerAnisotropy", "textureCompressionETC2",
        "textureCompressionASTC_LDR", "textureCompressionBC", "occlusionQueryPrecise",
        "pipelineStatisticsQuery", "vertexPipelineStoresAndAtomics", "fragmentStoresAndAtomics",
        "shaderTessellationAndGeometryPointSize", "shaderImageGatherExtended", "shaderStorageImageExtendedFormats",
        "shaderStorageImageMultisample", "shaderStorageImageReadWithoutFormat", "shaderStorageImageWriteWithoutFormat",
        "shaderUniformBufferArrayDynamicIndexing", "shaderSampledImageArrayDynamicIndexing", "shaderStorageBufferArrayDynamicIndexing",
        "shaderStorageImageArrayDynamicIndexing", "shaderClipDistance",  "shaderCullDistance",
        "shaderFloat64", "shaderInt64", "shaderInt16",
        "shaderResourceResidency", "shaderResourceMinLod", "sparseBinding",
        "sparseResidencyBuffer", "sparseResidencyImage2D", "sparseResidencyImage3D",
        "sparseResidency2Samples", "sparseResidency4Samples", "sparseResidency8Samples",
        "sparseResidency16Samples", "sparseResidencyAliased", "variableMultisampleRate",
        "inheritedQueries" 
    };
    std::map<const char*, int> physicalDeviceFeatures2Map = {
        { "robustBufferAccess", 0 }, { "fullDrawIndexUint32", 1 }, { "imageCubeArray", 2 },
        { "independentBlend", 3 }, { "geometryShader", 4 }, { "tessellationShader", 5 },
        { "sampleRateShading", 6 }, { "dualSrcBlend", 7 }, { "logicOp", 8 },
        { "multiDrawIndirect", 9 }, { "drawIndirectFirstInstance", 10 }, { "depthClamp", 11 },
        { "depthBiasClamp", 12 }, { "fillModeNonSolid", 13 }, { "depthBounds", 14 },
        { "wideLines", 15 }, { "largePoints", 16 }, { "alphaToOne", 17 },
        { "multiViewport", 18 }, { "samplerAnisotropy", 19 }, { "textureCompressionETC2", 20 },
        { "textureCompressionASTC_LDR", 21 }, { "textureCompressionBC", 22 }, { "occlusionQueryPrecise", 23 },
        { "pipelineStatisticsQuery", 24 }, { "vertexPipelineStoresAndAtomics", 25 }, { "fragmentStoresAndAtomics", 26 },
        { "shaderTessellationAndGeometryPointSize", 27 }, { "shaderImageGatherExtended", 28 }, { "shaderStorageImageExtendedFormats", 29 },
        { "shaderStorageImageMultisample", 30 }, { "shaderStorageImageReadWithoutFormat", 31 }, { "shaderStorageImageWriteWithoutFormat", 32 },
        { "shaderUniformBufferArrayDynamicIndexing", 33 }, { "shaderSampledImageArrayDynamicIndexing", 34 }, { "shaderStorageBufferArrayDynamicIndexing", 35 },
        { "shaderStorageImageArrayDynamicIndexing", 36 }, { "shaderClipDistance", 37 }, { "shaderCullDistance", 38 },
        { "shaderFloat64", 39 }, { "shaderInt64", 40 }, { "shaderInt16", 41 },
        { "shaderResourceResidency", 42 }, { "shaderResourceMinLod", 43 }, { "sparseBinding", 44 },
        { "sparseResidencyBuffer", 45 }, { "sparseResidencyImage2D", 46 }, { "sparseResidencyImage3D", 47 },
        { "sparseResidency2Samples", 48 }, { "sparseResidency4Samples", 49 }, { "sparseResidency8Samples", 50 },
        { "sparseResidency16Samples", 51 }, { "sparseResidencyAliased", 52 }, { "variableMultisampleRate", 53 },
        { "inheritedQueries", 54 } 
    };
    std::vector<const char*> physicalDeviceVulkan11FeaturesVector{
            "storageBuffer16BitAccess", "uniformAndStorageBuffer16BitAccess", "storagePushConstant16",
            "storageInputOutput16", "multiview", "multiviewGeometryShader",
            "multiviewTessellationShader", "variablePointersStorageBuffer", "variablePointers",
            "protectedMemory", "samplerYcbcrConversion", "shaderDrawParameters"
    };
    std::map<const char*, int> physicalDeviceVulkan11FeaturesMap = {
        { "storageBuffer16BitAccess", 0 }, { "uniformAndStorageBuffer16BitAccess", 1 }, { "storagePushConstant16", 2 },
        { "storageInputOutput16", 3 }, { "multiview", 4 }, { "multiviewGeometryShader", 5 },
        { "multiviewTessellationShader", 6 }, { "variablePointersStorageBuffer", 7 }, { "variablePointers", 8 },
        { "protectedMemory", 9 }, { "samplerYcbcrConversion", 10 }, { "shaderDrawParameters", 11 }
    };
    std::vector<const char*> physicalDeviceVulkan12FeaturesVector{
            "samplerMirrorClampToEdge", "drawIndirectCount", "storageBuffer8BitAccess",
            "uniformAndStorageBuffer8BitAccess", "storagePushConstant8", "shaderBufferInt64Atomics",
            "shaderSharedInt64Atomics", "shaderFloat16", "shaderInt8",
            "descriptorIndexing", "shaderInputAttachmentArrayDynamicIndexing", "shaderUniformTexelBufferArrayDynamicIndexing",
            "shaderStorageTexelBufferArrayDynamicIndexing", "shaderUniformBufferArrayNonUniformIndexing", "shaderSampledImageArrayNonUniformIndexing",
            "shaderStorageBufferArrayNonUniformIndexing", "shaderStorageImageArrayNonUniformIndexing", "shaderInputAttachmentArrayNonUniformIndexing",
            "shaderUniformTexelBufferArrayNonUniformIndexing", "shaderStorageTexelBufferArrayNonUniformIndexing", "descriptorBindingUniformBufferUpdateAfterBind",
            "descriptorBindingSampledImageUpdateAfterBind", "descriptorBindingStorageImageUpdateAfterBind", "descriptorBindingStorageBufferUpdateAfterBind",
            "descriptorBindingUniformTexelBufferUpdateAfterBind", "descriptorBindingStorageTexelBufferUpdateAfterBind", "descriptorBindingUpdateUnusedWhilePending",
            "descriptorBindingPartiallyBound", "descriptorBindingVariableDescriptorCount", "runtimeDescriptorArray",
            "samplerFilterMinmax", "scalarBlockLayout", "imagelessFramebuffer",
            "uniformBufferStandardLayout", "shaderSubgroupExtendedTypes", "separateDepthStencilLayouts",
            "hostQueryReset", "timelineSemaphore", "bufferDeviceAddress",
            "bufferDeviceAddressCaptureReplay", "bufferDeviceAddressMultiDevice", "vulkanMemoryModel",
            "vulkanMemoryModelDeviceScope", "vulkanMemoryModelAvailabilityVisibilityChains", "shaderOutputViewportIndex",
            "shaderOutputLayer", "subgroupBroadcastDynamicId"
    };
    std::map<const char*, int> physicalDeviceVulkan12FeaturesMap = {
        { "samplerMirrorClampToEdge", 0 }, { "drawIndirectCount", 1 }, { "storageBuffer8BitAccess", 2 },
        { "uniformAndStorageBuffer8BitAccess", 3 }, { "storagePushConstant8", 4 }, { "shaderBufferInt64Atomics", 5 },
        { "shaderSharedInt64Atomics", 6 }, { "shaderFloat16", 7 }, { "shaderInt8", 8 },
        { "descriptorIndexing", 9 }, { "shaderInputAttachmentArrayDynamicIndexing", 10 }, { "shaderUniformTexelBufferArrayDynamicIndexing", 11 },
        { "shaderStorageTexelBufferArrayDynamicIndexing", 12 }, { "shaderUniformBufferArrayNonUniformIndexing", 13 }, { "shaderSampledImageArrayNonUniformIndexing", 14 },
        { "shaderStorageBufferArrayNonUniformIndexing", 15 }, { "shaderStorageImageArrayNonUniformIndexing", 16 }, { "shaderInputAttachmentArrayNonUniformIndexing", 17 },
        { "shaderUniformTexelBufferArrayNonUniformIndexing", 18 }, { "shaderStorageTexelBufferArrayNonUniformIndexing", 19 }, { "descriptorBindingUniformBufferUpdateAfterBind", 20 },
        { "descriptorBindingSampledImageUpdateAfterBind", 21 }, { "descriptorBindingStorageImageUpdateAfterBind", 22 }, { "descriptorBindingStorageBufferUpdateAfterBind", 23 },
        { "descriptorBindingUniformTexelBufferUpdateAfterBind", 24 }, { "descriptorBindingStorageTexelBufferUpdateAfterBind", 25 }, { "descriptorBindingUpdateUnusedWhilePending", 26 },
        { "descriptorBindingPartiallyBound", 27 }, { "descriptorBindingVariableDescriptorCount", 28 }, { "runtimeDescriptorArray", 29 },
        { "samplerFilterMinmax", 30 }, { "scalarBlockLayout", 31 }, { "imagelessFramebuffer", 32 },
        { "uniformBufferStandardLayout", 33 }, { "shaderSubgroupExtendedTypes", 34 }, { "separateDepthStencilLayouts", 35 },
        { "hostQueryReset", 36 }, { "timelineSemaphore", 37 }, { "bufferDeviceAddress", 38 },
        { "bufferDeviceAddressCaptureReplay", 39 }, { "bufferDeviceAddressMultiDevice", 40 }, { "vulkanMemoryModel", 41 },
        { "vulkanMemoryModelDeviceScope", 42 }, { "vulkanMemoryModelAvailabilityVisibilityChains", 43 }, { "shaderOutputViewportIndex", 44 },
        { "shaderOutputLayer", 45 }, { "subgroupBroadcastDynamicId", 46 }
    };
    std::vector<const char*> physicalDeviceAccelerationStructureFeaturesKHRVector{
            "accelerationStructure", "accelerationStructureCaptureReplay", "accelerationStructureIndirectBuild",
            "accelerationStructureHostCommands", "descriptorBindingAccelerationStructureUpdateAfterBind"
    };
    std::map<const char*, int> physicalDeviceAccelerationStructureFeaturesKHRMap = {
        { "accelerationStructure", 0 }, { "accelerationStructureCaptureReplay", 1 }, { "accelerationStructureIndirectBuild", 2 },
        { "accelerationStructureHostCommands", 3 }, { "descriptorBindingAccelerationStructureUpdateAfterBind", 4 }
    };
    std::vector<const char*> physicalDeviceRayTracingPipelineFeaturesKHRVector{
            "rayTracingPipeline", "rayTracingPipelineShaderGroupHandleCaptureReplay", "rayTracingPipelineShaderGroupHandleCaptureReplayMixed",
            "rayTracingPipelineTraceRaysIndirect", "rayTraversalPrimitiveCulling"
    };
    std::map<const char*, int> physicalDeviceRayTracingPipelineFeaturesKHRMap = {
        { "rayTracingPipeline", 0 }, { "rayTracingPipelineShaderGroupHandleCaptureReplay", 1 }, { "rayTracingPipelineShaderGroupHandleCaptureReplayMixed", 2 },
        { "rayTracingPipelineTraceRaysIndirect", 3 }, { "rayTraversalPrimitiveCulling", 4 }
    };
    
};
}
#endif // !VULKAN_COMMON_UTILS
