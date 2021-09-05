#pragma once
#include "Constants.h"

namespace Initialisers {

	// Apps

	VkApplicationInfo applicationInfo(const char* name, const char* engineName, uint32_t appVersion = VK_MAKE_VERSION(1, 0, 0), uint32_t engineVersion = VK_MAKE_VERSION(1, 0, 0), uint32_t apiVersion = VK_API_VERSION_1_0);

	// Instances

	VkInstanceCreateInfo instanceCreateInfo(const VkApplicationInfo* pApplicationInfo, uint32_t enabledExtentionsCount, const char* const* ppEnabledExtensionName);
	
	// Surfaces

	// Devices

	// Descriptor Sets

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts);

	VkDescriptorBufferInfo descriptorBufferInfo(VkBuffer buffer, VkDeviceSize range, VkDeviceSize offset = 0);

	VkDescriptorImageInfo descriptorImageInfo(VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkWriteDescriptorSetAccelerationStructureKHR descriptorSetAccelerationStructureInfo(const VkAccelerationStructureKHR* pAccerlerationStructures, uint32_t accelerationStructureCount = 1);

	VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet descriptorSet, uint32_t descriptorBinding, VkDescriptorType descriptorType, const VkDescriptorBufferInfo* pBufferInfo, uint32_t descriptorCount = 1, uint32_t descriptorArrayElement = 0);
	
	VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet descriptorSet, uint32_t descriptorBinding, VkDescriptorType descriptorType, const VkDescriptorImageInfo* pImageInfo, uint32_t descriptorCount = 1, uint32_t descriptorArrayElement = 0);

	VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet descriptorSet, uint32_t descriptorBinding, VkDescriptorType descriptorType, const VkWriteDescriptorSetAccelerationStructureKHR* pAccelerationStructureInfo, uint32_t descriptorCount = 1, uint32_t descriptorArrayElement = 0);

	// Descriptor Pools

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets);

	VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount);

	// Descriptor Set Layouts

	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1, const VkSampler* pImmutableSamplers = nullptr);

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding* pBindings, uint32_t bindingCount);

	// Pipelines

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpasses, VkPipeline basePipelineHandle = VK_NULL_HANDLE);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount = 1);

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* pName);

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo();
	
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(const VkVertexInputBindingDescription* pVertexBindingDescriptions, uint32_t vertexBindingDescriptionCount, const VkVertexInputAttributeDescription* pVertexAttributeDescriptions, uint32_t vertexAttributeDescriptionCount);

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable = VK_FALSE);

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const VkViewport* pViewports, uint32_t viewportCount, const VkRect2D* pScissors, uint32_t scissorCount);

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontface, float lineWidth, VkBool32 depthClampEnable, VkBool32 depthBiasEnable = VK_FALSE, VkBool32 disacrdEnable = VK_FALSE);

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkBool32 sampleShadingEnable = VK_FALSE);

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthsBoundsTestEnable = VK_FALSE, VkBool32 stencilTestEanble = VK_FALSE);

	VkPipelineColorBlendAttachmentState pipelineColourBlendAttachmentState(VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VkBool32 blendEnable = VK_FALSE);

	VkPipelineColorBlendStateCreateInfo pipelineColourBlendStateCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachments, uint32_t attachmentCount, VkBool32 logicOpEnable, VkLogicOp logicOp, const std::vector<float>& blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f });

	VkPipelineColorBlendStateCreateInfo pipelineColourBlendStateCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachments, uint32_t attachmentCount);

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(const VkDynamicState* pDynamicStates, uint32_t dynamicStateCount);

	VkSpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size);

	VkSpecializationInfo specializationInfo(uint32_t mapEntryCount, const VkSpecializationMapEntry* mapEntries, size_t dataSize, const void* data);

	// Shaders

	VkShaderModuleCreateInfo shaderModuleCreateInfo(const uint32_t* pCode, size_t codeSize);

	// Command Buffers

	VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0U);

	// Buffers

	VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

	VkBufferCopy bufferCopy(VkDeviceSize size, VkDeviceSize srcOffset = 0U, VkDeviceSize dstOffset = 0U);

	// Images

	VkImageCreateInfo imageCreateInfo(VkImageType imageType,
		uint32_t width, uint32_t height, uint32_t depth,
		VkFormat format, VkImageUsageFlags usage,
		VkImageTiling tiling, VkSampleCountFlagBits samples, uint32_t mipLevels = 1,
		VkImageLayout initialialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		uint32_t arrayLayers = 1,
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

	VkBufferImageCopy bufferImageCopy(uint32_t width, uint32_t height, uint32_t depth = 1,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		VkDeviceSize bufferOffset = 0,
		uint32_t bufferRowLength = 0,
		uint32_t bufferImageHeight = 0,
		int32_t imageOffsetU = 0,
		int32_t imageOffsetV = 0,
		int32_t imageOffsetW = 0,
		uint32_t mipLevel = 0,
		uint32_t baseArrayLayer = 0,
		uint32_t layerCount = 1);

	VkImageCopy imageCopy(uint32_t width, uint32_t height, uint32_t depth = 1, 
		const VkImageSubresourceLayers& srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		const VkImageSubresourceLayers& dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 }, 
		const VkOffset3D& srcOffset = {0, 0, 0},
		const VkOffset3D& dstOffset = {0, 0, 0});

	VkImageCopy imageCopy(uint32_t width, uint32_t height, uint32_t depth,
		const VkOffset3D& srcOffset,
		const VkOffset3D& dstOffset,
		const VkImageSubresourceLayers& srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		const VkImageSubresourceLayers& dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 });

	//Image Views

	VkImageViewCreateInfo imageViewCreateInfo(VkImage image,
		VkImageViewType viewType, VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32_t baseMipLevel = 0,
		uint32_t levelCount = 1,
		uint32_t baseArrayLayer = 0,
		uint32_t layerCount = 1);

	VkImageViewCreateInfo imageViewCreateInfo(VkImage image, VkImageViewType viewType, VkFormat format);

	// Samplers

	VkSamplerCreateInfo samplerCreateInfo(VkFilter magFilter, VkFilter minFilter, VkBool32 anisotropyEnable, float maxAnisotropy, VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		VkBool32 unnormalizedCoordinates = VK_FALSE,
		VkBorderColor borderCollor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		VkBool32 compareEnable = VK_FALSE,
		VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS);

	VkSamplerCreateInfo samplerCreateInfo(VkFilter filter,  float maxAnisotropy, VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressMode,
		VkBorderColor borderColour);

	// SwapChains

	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR(VkSurfaceKHR surface,
		uint32_t minImageCount,
		VkFormat surfaceFormat, VkColorSpaceKHR colorSpace, VkExtent2D extent,
		VkSurfaceTransformFlagBitsKHR preTransform, VkPresentModeKHR presentMode,
		VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VkBool32 clipped = VK_TRUE,
		uint32_t imageArrayLayers = 1);

	// FrameBuffers

	VkFramebufferCreateInfo framebufferCreateInfo(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, VkExtent2D extent, int32_t layers = 1);

	// Render Passes

	VkRenderPassCreateInfo renderPassCreateInfo(uint32_t attachmentCount, const VkAttachmentDescription* pAttachments, 
		uint32_t subpassCount, const VkSubpassDescription* pSubpasses, 
		uint32_t dependencyCount, const VkSubpassDependency* pDependencies);

	VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, uint32_t clearValueCount, const VkClearValue* pClearValues, VkOffset2D offset = { 0, 0 });

	VkAttachmentDescription attachmentDescription(VkFormat format, VkSampleCountFlagBits samples, 
		VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, 
		VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, 
		VkImageLayout initialLayout, VkImageLayout finalLayout);

	VkAttachmentReference attachmentReference(uint32_t attachment, VkImageLayout layout);

	VkSubpassDescription subpassDescription(VkPipelineBindPoint pipelineBindPoint, uint32_t colorAttachmentCount,
		const VkAttachmentReference* pColorAttachments, const VkAttachmentReference* pDepthAttachments);

	VkSubpassDescription subpassDescription(VkPipelineBindPoint pipelineBindPoint);

	VkSubpassDependency subpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, 
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, 
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);

	// Semaphores

	VkSemaphoreCreateInfo semaphoreCreateInfo();

	// Fences

	VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = VK_FENCE_CREATE_SIGNALED_BIT);

	// Barriers

	VkImageMemoryBarrier imageMemoryBarrier(VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		uint32_t baseMipLevel = 0,
		uint32_t levelCount = 1,
		uint32_t baseArrayLayer = 0,
		uint32_t layerCount = 1);

	VkImageMemoryBarrier imageMemoryBarrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, const VkImageSubresourceRange& subresourceRange);

	VkImageMemoryBarrier imageMemoryBarrier();

	// Present

	VkPresentInfoKHR presentInfoKHR(const VkSemaphore* pWaitSemaphores, uint32_t waitSemaphoreCount, const VkSwapchainKHR* swapChains, uint32_t swapChainCount, const uint32_t* pImageIndices);

	// Submit

	VkSubmitInfo submitInfo(const VkCommandBuffer* pCommandBuffers, uint32_t commandBufferCount,
		const VkSemaphore* pWaitSemaphores = nullptr, uint32_t waitSemaphoreCount = 0,
		const VkSemaphore* pSignalSemaphores = nullptr, uint32_t signalSemaphoreCount = 0,
		const VkPipelineStageFlags* pWaitDstStageMask = nullptr);

	// Memory

	VkMemoryAllocateInfo memoryAllocateInfo(VkDeviceSize allocationSize,
		uint32_t memoryTypeIndex = 0U);

	// Viewport 

	VkViewport viewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

	VkRect2D scissor(VkExtent2D extent, VkExtent2D offset = { 0, 0 });

	// Vertices

	VkVertexInputBindingDescription vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

	VkVertexInputAttributeDescription vertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);

	// Ray Tracing Pipeline

	VkRayTracingPipelineCreateInfoKHR RayTracingPipelineCreateInfo(VkPipelineLayout pipelineLayout, const VkPipelineShaderStageCreateInfo* shaderStages, uint32_t stageCount, const VkRayTracingShaderGroupCreateInfoKHR* shaderGroups, uint32_t groupCount, uint32_t maxRecursionDepth = 1);

	VkRayTracingPipelineCreateInfoKHR RayTracingPipelineCreateInfo(uint32_t maxRecursionDepth = 1);

	// Ray Tracing Shader

	VkRayTracingShaderGroupCreateInfoKHR rayTracingGeneralShaderGroup(uint32_t shaderCount);

	VkRayTracingShaderGroupCreateInfoKHR rayTracingClosestHitShaderGroup(uint32_t shaderCount);

	// Acceleration Structure

	VkAccelerationStructureCreateInfoKHR ASCreateInfo(VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset, VkAccelerationStructureTypeKHR type);

	VkAccelerationStructureGeometryTrianglesDataKHR ASGTriangleData(const VkDeviceOrHostAddressConstKHR& vertexData, VkDeviceSize vertexStride, uint32_t maxVertex, VkDeviceOrHostAddressConstKHR indexData, VkDeviceOrHostAddressConstKHR transformData);

	VkAccelerationStructureGeometryInstancesDataKHR ASGInstanceData(const VkDeviceOrHostAddressConstKHR& instanceData, VkBool32 arrayOfPointers = VK_FALSE);

	VkAccelerationStructureGeometryKHR ASG(const VkAccelerationStructureGeometryTrianglesDataKHR& triangles);

	VkAccelerationStructureGeometryKHR ASG(const VkAccelerationStructureGeometryInstancesDataKHR& instances);

	VkAccelerationStructureBuildSizesInfoKHR ASBuildSizesInfo();

	VkAccelerationStructureBuildGeometryInfoKHR BLABuildGeometryInfo(const VkAccelerationStructureGeometryKHR* pGeometries, uint32_t geometryCount = 1, VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	VkAccelerationStructureBuildGeometryInfoKHR BLABuildGeometryInfo(const VkAccelerationStructureKHR& dstAccelerationStructure, const VkAccelerationStructureGeometryKHR* pGeometries, uint32_t geometryCount = 1, VkDeviceAddress scratchBufferAddress = 0U, VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

	VkAccelerationStructureBuildGeometryInfoKHR ABuildGeometryInfo(const VkAccelerationStructureBuildGeometryInfoKHR& prevAccelerationBuildGeometryInfo, const VkAccelerationStructureKHR& dstAccelerationStructure, VkDeviceAddress scratchBufferAddress = 0U);

	VkAccelerationStructureBuildGeometryInfoKHR BLABuildGeometryInfo(VkAccelerationStructureKHR dstAccelerationStructure,
		VkDeviceAddress scratchBufferAddress, const VkAccelerationStructureGeometryKHR* pGeometries, uint32_t geometryCount = 1);

	VkAccelerationStructureBuildGeometryInfoKHR TLABuildGeometryInfo(const VkAccelerationStructureGeometryKHR* pGeometries, uint32_t geometryCount = 1);

	VkAccelerationStructureBuildRangeInfoKHR ASBuildRangeInfo(uint32_t primitiveCount, uint32_t primitiveOffset = 0, uint32_t firstVertex = 0, uint32_t transformOffset = 0);

	VkAccelerationStructureInstanceKHR ASInstance(VkTransformMatrixKHR transformMatrix, VkGeometryInstanceFlagsKHR flags, uint64_t accelerationStructureReference, uint32_t instanceCustomIndex, uint32_t instanceShaderBindingTableRecordOffset = 0, uint32_t mask = 0xFF);

	VkAccelerationStructureDeviceAddressInfoKHR ASDeviceAddressInfo(const VkAccelerationStructureKHR& accelerationStructure);

	// Device Address

	VkStridedDeviceAddressRegionKHR StridedDeviceAddressRegion(VkDeviceAddress deviceAddress, VkDeviceSize size, VkDeviceSize stride);
}
