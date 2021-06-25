#include "Initilizers.h"

namespace Initialisers {

	// Apps

	VkApplicationInfo applicationInfo(const char* name, const char* engineName, uint32_t appVersion, uint32_t engineVersion, uint32_t apiVersion) {
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = appVersion;
		appInfo.pEngineName = engineName;
		appInfo.engineVersion = engineVersion;
		appInfo.apiVersion = apiVersion;
		return appInfo;
	}

	// Instances

	VkInstanceCreateInfo instanceCreateInfo(const VkApplicationInfo* pApplicationInfo, uint32_t enabledExtentionsCount, const char* const* ppEnabledExtensionName) {
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = pApplicationInfo;
		createInfo.enabledExtensionCount = enabledExtentionsCount;
		createInfo.ppEnabledExtensionNames = ppEnabledExtensionName;
		return createInfo;
	}

	// Surfaces

	// Devices

	// Descriptor Sets

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSetLayout* pSetLayouts) {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = descriptorSetCount;
		allocInfo.pSetLayouts = pSetLayouts;
		return allocInfo;
	}

	VkDescriptorBufferInfo descriptorBufferInfo(VkBuffer buffer, VkDeviceSize range, VkDeviceSize offset) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = offset;
		bufferInfo.range = range;
		return bufferInfo;
	}

	VkDescriptorImageInfo descriptorImageInfo(VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = imageLayout;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;
		return imageInfo;
	}


	VkWriteDescriptorSet writeBufferDescriptorSet(VkDescriptorSet descriptorSet, uint32_t descriptorBinding, VkDescriptorType descriptorType, uint32_t descriptorCount, const VkDescriptorBufferInfo* pBufferInfo, uint32_t descriptorArrayElement) {

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = descriptorBinding;
		descriptorWrite.dstArrayElement = descriptorArrayElement;
		descriptorWrite.descriptorType = descriptorType;
		descriptorWrite.descriptorCount = descriptorCount;
		descriptorWrite.pBufferInfo = pBufferInfo;
		return descriptorWrite;
	}

	VkWriteDescriptorSet writeImageDescriptorSet(VkDescriptorSet descriptorSet, uint32_t descriptorBinding, VkDescriptorType descriptorType, uint32_t descriptorCount, const VkDescriptorImageInfo* pImageInfo, uint32_t descriptorArrayElement) {

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = descriptorBinding;
		//descriptorWrite.dstArrayElement = descriptorArrayElement;
		descriptorWrite.descriptorType = descriptorType;
		descriptorWrite.descriptorCount = descriptorCount;
		descriptorWrite.pImageInfo = pImageInfo;
		return descriptorWrite;
	}

	// Descriptor Pools

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(uint32_t poolSizeCount, const VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets) {
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizeCount;
		poolInfo.pPoolSizes = pPoolSizes;
		poolInfo.maxSets = maxSets;
		return poolInfo;
	}

	VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount) {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = type;
		poolSize.descriptorCount = descriptorCount;
		return poolSize;
	}

	// Descriptor Set Layouts

	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount, const VkSampler* pImmutableSamplers) {
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = descriptorCount;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.pImmutableSamplers = pImmutableSamplers;
		layoutBinding.stageFlags = stageFlags;
		return layoutBinding;
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding* pBindings, uint32_t bindingCount) {
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindingCount;
		layoutInfo.pBindings = pBindings;
		return layoutInfo;
	}

	// Pipelines

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, uint32_t subpasses, VkPipeline basePipelineHandle) {
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = layout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = subpasses;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		return pipelineInfo;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts, uint32_t setLayoutCount) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = setLayoutCount;
		pipelineLayoutInfo.pSetLayouts = pSetLayouts;
		return pipelineLayoutInfo;
	}

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* pName) {

		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = stage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = pName;
		return shaderStageInfo;
	}

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() {
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		return vertexInputInfo;
	}

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(const VkVertexInputBindingDescription* pVertexBindingDescriptions, uint32_t vertexBindingDescriptionCount, const VkVertexInputAttributeDescription* pVertexAttributeDescriptions, uint32_t vertexAttributeDescriptionCount) {
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = vertexBindingDescriptionCount;
		vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
		vertexInputInfo.pVertexBindingDescriptions = pVertexBindingDescriptions;
		vertexInputInfo.pVertexAttributeDescriptions = pVertexAttributeDescriptions;
		return vertexInputInfo;
	}

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable) {
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = topology;
		inputAssembly.primitiveRestartEnable = primitiveRestartEnable;
		return inputAssembly;
	}

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(const VkViewport* pViewports, uint32_t viewportCount, const VkRect2D* pScissors, uint32_t scissorCount) {
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = viewportCount;
		viewportState.pViewports = pViewports;
		viewportState.scissorCount = scissorCount;
		viewportState.pScissors = pScissors;
		return viewportState;
	}

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontface, float lineWidth, VkBool32 depthClampEnable, VkBool32 depthBiasEnable, VkBool32 disacrdEnable) {
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = depthClampEnable;
		rasterizer.rasterizerDiscardEnable = disacrdEnable;
		rasterizer.polygonMode = polygonMode;
		rasterizer.lineWidth = lineWidth;
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = frontface;
		rasterizer.depthBiasEnable = depthBiasEnable;
		return rasterizer;
	}

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(VkSampleCountFlagBits samples, VkBool32 sampleShadingEnable) {
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = sampleShadingEnable;
		multisampling.rasterizationSamples = samples;
		return multisampling;
	}

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp, VkBool32 depthsBoundsTestEnable, VkBool32 stencilTestEanble) {
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = depthTestEnable;
		depthStencil.depthWriteEnable = depthWriteEnable;
		depthStencil.depthCompareOp = depthCompareOp;
		depthStencil.depthBoundsTestEnable = depthsBoundsTestEnable;
		depthStencil.stencilTestEnable = stencilTestEanble;
		return depthStencil;
	}

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask, VkBool32 blendEnable) {
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = colorWriteMask;
		colorBlendAttachment.blendEnable = blendEnable;
		return colorBlendAttachment;
	}

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(const VkPipelineColorBlendAttachmentState* pAttachments, uint32_t attachmentCount, VkBool32 logicOpEnable, VkLogicOp logicOp, const std::vector<float>& blendConstants) {
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = logicOpEnable;
		colorBlending.logicOp = logicOp;
		colorBlending.attachmentCount = attachmentCount;
		colorBlending.pAttachments = pAttachments;
		colorBlending.blendConstants[0] = blendConstants[0];
		colorBlending.blendConstants[1] = blendConstants[1];
		colorBlending.blendConstants[2] = blendConstants[2];
		colorBlending.blendConstants[3] = blendConstants[3];
		return colorBlending;
	}

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(const VkDynamicState* pDynamicStates, uint32_t dynamicStateCount) {
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = pDynamicStates;
		dynamicState.dynamicStateCount = dynamicStateCount;
		return dynamicState;
	}

	VkSpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
	{
		VkSpecializationMapEntry specializationMapEntry{};
		specializationMapEntry.constantID = constantID;
		specializationMapEntry.offset = offset;
		specializationMapEntry.size = size;
		return specializationMapEntry;
	}

	VkSpecializationInfo specializationInfo(uint32_t mapEntryCount, const VkSpecializationMapEntry* mapEntries, size_t dataSize, const void* data)
	{
		VkSpecializationInfo specializationInfo{};
		specializationInfo.mapEntryCount = mapEntryCount;
		specializationInfo.pMapEntries = mapEntries;
		specializationInfo.dataSize = dataSize;
		specializationInfo.pData = data;
		return specializationInfo;
	}

	// Shaders

	VkShaderModuleCreateInfo shaderModuleCreateInfo(const uint32_t* pCode, size_t codeSize) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = codeSize;
		createInfo.pCode = pCode;
		return createInfo;
	}

	// Command Buffers

	VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBufferLevel level) {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = level;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = commandBufferCount;
		return allocInfo;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flags;
		return beginInfo;
	}

	// Buffers

	VkBufferCreateInfo bufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = sharingMode;
		return bufferInfo;
	}

	VkBufferCopy bufferCopy(VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) {
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		copyRegion.dstOffset = dstOffset;
		copyRegion.srcOffset = srcOffset;
		return copyRegion;
	}

	// Images

	VkImageCreateInfo imageCreateInfo(VkImageType imageType,
		uint32_t width, uint32_t height, uint32_t depth,
		VkFormat format, VkImageUsageFlags usage,
		VkImageTiling tiling, VkSampleCountFlagBits samples, uint32_t mipLevels,
		VkImageLayout initialialLayout,
		uint32_t arrayLayers,
		VkSharingMode sharingMode) {

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = depth;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = initialialLayout;
		imageInfo.usage = usage;
		imageInfo.samples = samples;
		imageInfo.sharingMode = sharingMode;
		return imageInfo;
	}

	VkBufferImageCopy bufferImageCopy(uint32_t width, uint32_t height, uint32_t depth,
		VkImageAspectFlags aspectMask,
		VkDeviceSize bufferOffset,
		uint32_t bufferRowLength,
		uint32_t bufferImageHeight,
		int32_t imageOffsetU,
		int32_t imageOffsetV,
		int32_t imageOffsetW,
		uint32_t mipLevel,
		uint32_t baseArrayLayer,
		uint32_t layerCount) {

		VkBufferImageCopy region{};
		region.bufferOffset = bufferOffset;
		region.bufferRowLength = bufferRowLength;
		region.bufferImageHeight = bufferImageHeight;
		region.imageSubresource.aspectMask = aspectMask;
		region.imageSubresource.mipLevel = mipLevel;
		region.imageSubresource.baseArrayLayer = baseArrayLayer;
		region.imageSubresource.layerCount = layerCount;
		region.imageOffset = { imageOffsetU, imageOffsetV, imageOffsetW };
		region.imageExtent = {
			width,
			height,
			depth
		};
		return region;
	}

	//Image Views

	VkImageViewCreateInfo imageViewCreateInfo(VkImage image,
		VkImageViewType viewType, VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32_t baseMipLevel,
		uint32_t levelCount,
		uint32_t baseArrayLayer,
		uint32_t layerCount) {

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = viewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = baseMipLevel;
		viewInfo.subresourceRange.levelCount = levelCount;
		viewInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
		viewInfo.subresourceRange.layerCount = layerCount;
		return viewInfo;
	}

	// Samplers

	VkSamplerCreateInfo samplerCreateInfo(VkFilter magFilter, VkFilter minFilter, VkBool32 anisotropyEnable, float maxAnisotropy, VkSamplerMipmapMode mipmapMode,
		VkSamplerAddressMode addressModeU,
		VkSamplerAddressMode addressModeV,
		VkSamplerAddressMode addressModeW,
		VkBool32 unnormalizedCoordinates,
		VkBorderColor borderCollor,
		VkBool32 compareEnable,
		VkCompareOp compareOp) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = addressModeU;
		samplerInfo.addressModeV = addressModeV;
		samplerInfo.addressModeW = addressModeW;
		samplerInfo.anisotropyEnable = anisotropyEnable;
		samplerInfo.maxAnisotropy = maxAnisotropy;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.unnormalizedCoordinates = unnormalizedCoordinates;
		samplerInfo.borderColor = borderCollor;
		samplerInfo.compareEnable = compareEnable;
		samplerInfo.compareOp = compareOp;
		return samplerInfo;
	}

	VkSamplerCreateInfo samplerCreateInfo(VkFilter filter, float maxAnisotropy, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode, VkBorderColor borderColour)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.maxAnisotropy = maxAnisotropy;
		samplerInfo.mipmapMode = mipmapMode;
		samplerInfo.borderColor = borderColour;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f; 
		return samplerInfo;
	}

	// Swapchains

	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR(VkSurfaceKHR surface,
		uint32_t minImageCount,
		VkFormat surfaceFormat, VkColorSpaceKHR colorSpace, VkExtent2D extent,
		VkSurfaceTransformFlagBitsKHR preTransform, VkPresentModeKHR presentMode,
		VkImageUsageFlags imageUsage,
		VkCompositeAlphaFlagBitsKHR compositeAlpha,
		VkBool32 clipped,
		uint32_t imageArrayLayers) {

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = minImageCount;
		createInfo.imageFormat = surfaceFormat;
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = extent;
		createInfo.preTransform = preTransform;
		createInfo.presentMode = presentMode;
		createInfo.imageUsage = imageUsage;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.clipped = clipped;
		createInfo.imageArrayLayers = imageArrayLayers;
		return createInfo;
	}

	// Frame Buffers

	VkFramebufferCreateInfo framebufferCreateInfo(VkRenderPass renderPass, const VkImageView* pAttachments, uint32_t attachmentCount, VkExtent2D extent, int32_t layers) {
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachmentCount;
		framebufferInfo.pAttachments = pAttachments;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = layers;
		return framebufferInfo;
	}

	// Render Passes

	VkRenderPassCreateInfo renderPassCreateInfo(uint32_t attachmentCount, const VkAttachmentDescription* pAttachments, 
		uint32_t subpassCount, const VkSubpassDescription* pSubpasses, 
		uint32_t dependencyCount, const VkSubpassDependency* pDependencies) {
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentCount;
		renderPassInfo.pAttachments = pAttachments;
		renderPassInfo.subpassCount = subpassCount;
		renderPassInfo.pSubpasses = pSubpasses;
		renderPassInfo.dependencyCount = dependencyCount;
		renderPassInfo.pDependencies = pDependencies;
		return renderPassInfo;
	}


	VkRenderPassBeginInfo renderPassBeginInfo(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, uint32_t clearValueCount, const VkClearValue* pClearValues, VkOffset2D offset) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffer;
		renderPassInfo.renderArea.offset = offset;
		renderPassInfo.renderArea.extent = extent;
		renderPassInfo.clearValueCount = clearValueCount;
		renderPassInfo.pClearValues = pClearValues;
		return renderPassInfo;
	}

	VkAttachmentDescription attachmentDescription(VkFormat format, VkSampleCountFlagBits samples, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, 
		VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp, VkImageLayout initialLayout, VkImageLayout finalLayout) {
		VkAttachmentDescription attachment{};
		attachment.format = format;
		attachment.samples = samples;
		attachment.loadOp = loadOp;
		attachment.storeOp = storeOp;
		attachment.stencilLoadOp = stencilLoadOp;
		attachment.stencilStoreOp = stencilStoreOp;
		attachment.initialLayout = initialLayout;
		attachment.finalLayout = finalLayout;
		return attachment;
	}

	VkAttachmentReference attachmentReference(uint32_t attachment, VkImageLayout layout) {
		VkAttachmentReference attachmentReference{};
		attachmentReference.attachment = attachment;
		attachmentReference.layout = layout;
		return attachmentReference;
	}

	VkSubpassDescription subpassDescription(VkPipelineBindPoint pipelineBindPoint, uint32_t colorAttachmentCount,
		const VkAttachmentReference* pColorAttachments, const VkAttachmentReference* pDepthAttachments) {
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = pipelineBindPoint;
		subpass.colorAttachmentCount = colorAttachmentCount;
		subpass.pColorAttachments = pColorAttachments;
		subpass.pDepthStencilAttachment = pDepthAttachments;
		return subpass;
	}

	VkSubpassDependency subpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, 
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask) {
		VkSubpassDependency dependency{};
		dependency.srcSubpass = srcSubpass;
		dependency.dstSubpass = dstSubpass;
		dependency.srcStageMask = srcStageMask;
		dependency.dstStageMask = dstStageMask;
		dependency.srcAccessMask = srcAccessMask;
		dependency.dstAccessMask = dstAccessMask;
		return dependency;
	}

	// Semaphores

	VkSemaphoreCreateInfo semaphoreCreateInfo() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		return semaphoreInfo;
	}

	// Fences

	VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags) {
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = flags;
		return fenceInfo;
	}

	// Barriers

	VkImageMemoryBarrier imageMemoryBarrier(VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
		VkImageAspectFlags aspectMask,
		uint32_t baseMipLevel,
		uint32_t levelCount,
		uint32_t baseArrayLayer,
		uint32_t layerCount) {

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = levelCount;
		barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
		barrier.subresourceRange.layerCount = layerCount;
		return barrier;
	}

	// Present

	VkPresentInfoKHR presentInfoKHR(const VkSemaphore* pWaitSemaphores, uint32_t waitSemaphoreCount, const VkSwapchainKHR* swapChains, uint32_t swapChainCount, const uint32_t* pImageIndices) {
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = waitSemaphoreCount;
		presentInfo.pWaitSemaphores = pWaitSemaphores;
		presentInfo.swapchainCount = swapChainCount;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = pImageIndices;
		return presentInfo;
	}

	// Submit

	VkSubmitInfo submitInfo(const VkCommandBuffer* pCommandBuffers, uint32_t commandBufferCount,
		const VkSemaphore* pWaitSemaphores, uint32_t waitSemaphoreCount,
		const VkSemaphore* pSignalSemaphores, uint32_t signalSemaphoreCount,
		const VkPipelineStageFlags* pWaitDstStageMask) {

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		submitInfo.pWaitSemaphores = pWaitSemaphores;
		submitInfo.pWaitDstStageMask = pWaitDstStageMask;
		submitInfo.commandBufferCount = commandBufferCount;
		submitInfo.pCommandBuffers = pCommandBuffers;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores = pSignalSemaphores;
		return submitInfo;
	}

	// Memory

	VkMemoryAllocateInfo memoryAllocateInfo(VkDeviceSize allocationSize,
		uint32_t memoryTypeIndex) {
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = allocationSize;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		return allocInfo;
	}

	// Viewport 

	VkViewport viewport(float x, float y, float width, float height, float minDepth, float maxDepth) {
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;
		return viewport;
	}

	VkRect2D scissor(VkExtent2D extent, VkExtent2D offset) {
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		return scissor;
	}

	// Vertices

	VkVertexInputBindingDescription vertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate) {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = binding;
		bindingDescription.stride = stride;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	VkVertexInputAttributeDescription vertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset) {
		VkVertexInputAttributeDescription attributeDescription{};

		attributeDescription.binding = binding;
		attributeDescription.location = location;
		attributeDescription.format = format;
		attributeDescription.offset = offset;
		return attributeDescription;
	}

}